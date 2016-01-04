/*
 *  MIT License
 *  Copyright 2005-2016 Buaabyl
 *
 *  (http://opensource.org/licenses/MIT)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef TARGET_LINUX
#error This file just support linux
#endif

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "http_common.h"
#include "http_sys.h"

////////////////////////////////////////////////////////////
typedef struct _http_memory_t {
    void*   p;
    int     size;

    //reference to const string, don't modify!
    char*   file;
    int     line;
}_http_memory_t;

typedef enum {
    _HTTP_TYPE_MUTEX = 0,
    _HTTP_TYPE_EVENT = 1,
}_http_handle_type_t;

struct http_sys_handle_t
{
    _http_handle_type_t type;
    pthread_mutex_t mutex;
    sem_t event;
};

static _http_memory_t _memory_map[10 * 1024];
static const int _memory_len = sizeof(_memory_map)/sizeof(_memory_map[0]);
static pthread_mutex_t _memory_mutex;

////////////////////////////////////////////////////////////
void http_init(void)
{
    memset(_memory_map, 0, sizeof(_memory_map));
    pthread_mutex_init(&_memory_mutex, NULL);
}

void http_fini(void)
{
    pthread_mutex_destroy(&_memory_mutex);
}

int http_memory_usage(void)
{
    int i;
    int size = 0;

    if (pthread_mutex_lock(&_memory_mutex) != 0) {
        return -1;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == NULL) {
            continue;
        }
        size += _memory_map[i].size;
    }

    pthread_mutex_unlock(&_memory_mutex);

    return size;
}

void http_memory_dump(void)
{
    int i;

    if (pthread_mutex_lock(&_memory_mutex) != 0) {
        return;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == NULL) {
            continue;
        }
        printf(" 0x%p: %8d bytes, %s:%d\n",
                _memory_map[i].p,
                _memory_map[i].size,
                _memory_map[i].file,
                _memory_map[i].line);
    }

    pthread_mutex_unlock(&_memory_mutex);
}

void* http_malloc_(int size, char* file, int line)
{
    int i;
    void* p;

    if (pthread_mutex_lock(&_memory_mutex) != 0) {
        return NULL;
    }

    p = malloc(size);

    //add to memory map
    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == NULL) {
            break;
        }
    }

    if (i == _memory_len) {
        printf("Error: memory map full!\n");
        pthread_mutex_unlock(&_memory_mutex);
        exit(-1);
    }

    _memory_map[i].p = p;
    _memory_map[i].size = size;
    _memory_map[i].file = file;
    _memory_map[i].line = line;

    pthread_mutex_unlock(&_memory_mutex);

    return p;
}

void* http_realloc_(void* p, int size, char* file, int line)
{
    int i;
    void* new_p;

    if (pthread_mutex_lock(&_memory_mutex) != 0) {
        return NULL;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == p) {
            break;
        }
    }

    if (i == _memory_len) {
        printf("Error: realloc %p not exist in memory map!\n", p);
        printf(" from %s:%d\n", file, line);
        pthread_mutex_unlock(&_memory_mutex);
        exit(-1);
    }

    new_p = realloc(p, size);
    _memory_map[i].p = new_p;
    _memory_map[i].size = size;
    _memory_map[i].file = file;
    _memory_map[i].line = line;
    pthread_mutex_unlock(&_memory_mutex);

    return new_p;
}

void http_free_(void* p, char* file, int line)
{
    int i;

    if (p == NULL) {
        return;
    }

    if (pthread_mutex_lock(&_memory_mutex) != 0) {
        return;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == p) {
            break;
        }
    }

    if (i == _memory_len) {
        printf("Error: free %p not exist in memory map!\n", p);
        printf(" from %s:%d\n", file, line);
        exit(-1);
    }

    _memory_map[i].p = NULL;
    _memory_map[i].size = 0;

    free(p);

    pthread_mutex_unlock(&_memory_mutex);
}

http_sys_handle_t* http_mutex_new(void)
{
    http_sys_handle_t* h;

    h = (http_sys_handle_t*)http_malloc(sizeof(http_sys_handle_t));
    h->type = _HTTP_TYPE_MUTEX;
    pthread_mutex_init(&h->mutex, NULL);

    return h;
}

void http_mutex_delete(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return;
    }

    pthread_mutex_destroy(&h->mutex);

    http_free(h);
}

http_bool_t http_mutex_take(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return HTTP_FALSE;
    }

    if (pthread_mutex_lock(&h->mutex) != 0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

void http_mutex_give(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return;
    }
    pthread_mutex_unlock(&h->mutex);
}

http_sys_handle_t* http_event_new(void)
{
    http_sys_handle_t* h;

    h = (http_sys_handle_t*)http_malloc(sizeof(http_sys_handle_t));
    h->type = _HTTP_TYPE_EVENT;
    sem_init(&h->event, 0, 0);

    return h;
}

void http_event_delete(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return;
    }

    sem_destroy(&h->event);

    http_free(h);
}

http_bool_t http_event_trywait(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (sem_trywait(&h->event) != 0) {
        return HTTP_FALSE;
    }

    return HTTP_TRUE;
}

http_bool_t http_event_wait(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (sem_wait(&h->event) != 0) {
        return HTTP_FALSE;
    }

    return HTTP_TRUE;
}

http_bool_t http_event_waittimeout(http_sys_handle_t* h, int tmo)
{
    struct timespec t;
    long ovf_sec;
    long tv_nsec;

    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (clock_gettime(CLOCK_REALTIME, &t) != 0) {
        printf("clock_gettime error\n");
    }

    tv_nsec = t.tv_nsec + (tmo % 1000) * 1000 * 1000;
    ovf_sec = tv_nsec/1000/1000/1000;
    if (ovf_sec > 0) {
        t.tv_sec++;
        tv_nsec = tv_nsec - ovf_sec*1000*1000*1000;
    }
    t.tv_sec += tmo / 1000;
    t.tv_nsec = tv_nsec;

    if (sem_timedwait(&h->event, &t) != 0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

void http_event_set(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return;
    }
    sem_post(&h->event);
}

void http_msleep(int ms)
{
    usleep(ms * 1000);
}

typedef void* (*_pthread_callback_t)(void*);

void http_begin_thread(http_thread_proc_t f, void* param)
{
    pthread_t tid;
    int ret;

    ret = pthread_create(&tid, NULL, (_pthread_callback_t)f, param);
    if (ret != 0) {
        if (ret == EAGAIN) {
            printf("pthread_create: the system lacked the necessary resources!\n");
        } else {
            printf("pthread_create return %d\n", ret);
        }
    }
    pthread_detach(tid);
}

void http_end_thread(void)
{
    pthread_exit(NULL);
}

