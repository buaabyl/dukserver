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

#ifndef TARGET_WINDOWS
#error This file just support windows
#endif

#include <windows.h>
#include <process.h>
#include "http_common.h"
#include "http_sys.h"

////////////////////////////////////////////////////////////
typedef struct _http_memory_t {
    //remember pointer and size
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

typedef struct http_sys_handle_t
{
    _http_handle_type_t type;
    HANDLE mutex;
    HANDLE event;
}http_sys_handle_t;

//this for memory alloc and free
//because http server running in multithread mode,
//so memory must lock before any modify!
//
//this record memory pointer for leak checking
static _http_memory_t _memory_map[10 * 1024];
static const int _memory_len = sizeof(_memory_map)/sizeof(_memory_map[0]);
static HANDLE _memory_mutex = INVALID_HANDLE_VALUE;

////////////////////////////////////////////////////////////
void http_init(void)
{
    memset(_memory_map, 0, sizeof(_memory_map));
    _memory_mutex = CreateMutex(NULL, FALSE, NULL);
}

void http_fini(void)
{
    if (_memory_mutex != INVALID_HANDLE_VALUE) {
        CloseHandle(_memory_mutex);
        _memory_mutex = INVALID_HANDLE_VALUE;
    }
}

int http_memory_usage(void)
{
    int i;
    int size = 0;

    if (WaitForSingleObject(_memory_mutex, INFINITE) != WAIT_OBJECT_0) {
        return -1;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == NULL) {
            continue;
        }
        size += _memory_map[i].size;
    }

    ReleaseMutex(_memory_mutex);

    return size;
}

void http_memory_dump(void)
{
    int i;

    if (WaitForSingleObject(_memory_mutex, INFINITE) != WAIT_OBJECT_0) {
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

    ReleaseMutex(_memory_mutex);
}

void* http_malloc_(int size, char* file, int line)
{
    int i;
    void* p;

    if (WaitForSingleObject(_memory_mutex, INFINITE) != WAIT_OBJECT_0) {
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
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
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
        ReleaseMutex(_memory_mutex);
        exit(-1);
    }

    _memory_map[i].p = p;
    _memory_map[i].size = size;
    _memory_map[i].file = file;
    _memory_map[i].line = line;

    ReleaseMutex(_memory_mutex);

    return p;
}

void* http_realloc_(void* p, int size, char* file, int line)
{
    int i;
    void* new_p;

    if (WaitForSingleObject(_memory_mutex, INFINITE) != WAIT_OBJECT_0) {
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
        return NULL;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == p) {
            break;
        }
    }

    if (i == _memory_len) {
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
        ReleaseMutex(_memory_mutex);
        exit(-1);
    }

    new_p = realloc(p, size);
    _memory_map[i].p = new_p;
    _memory_map[i].size = size;
    _memory_map[i].file = file;
    _memory_map[i].line = line;

    ReleaseMutex(_memory_mutex);

    return new_p;
}

void http_free_(void* p, char* file, int line)
{
    int i;

    if (p == NULL) {
        return;
    }

    if (WaitForSingleObject(_memory_mutex, INFINITE) != WAIT_OBJECT_0) {
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
        return;
    }

    for (i = 0;i < _memory_len;i++) {
        if (_memory_map[i].p == p) {
            break;
        }
    }

    if (i == _memory_len) {
        printf("ERROR %s:%d\n", __FILE__, __LINE__);
        ReleaseMutex(_memory_mutex);
        exit(-1);
    }

    _memory_map[i].p = NULL;
    _memory_map[i].size = 0;

    free(p);

    ReleaseMutex(_memory_mutex);
}

http_sys_handle_t* http_mutex_new(void)
{
    http_sys_handle_t* h;

    h = (http_sys_handle_t*)http_malloc(sizeof(http_sys_handle_t));
    h->type = _HTTP_TYPE_MUTEX;
    h->mutex= CreateMutex(NULL, FALSE, NULL);

    return h;
}

void http_mutex_delete(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return;
    }

    CloseHandle(h->mutex);
    h->mutex = INVALID_HANDLE_VALUE;

    http_free(h);
}

http_bool_t http_mutex_take(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return HTTP_FALSE;
    }

    if (WaitForSingleObject(h->mutex, INFINITE) != WAIT_OBJECT_0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

void http_mutex_give(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_MUTEX)) {
        return;
    }
    ReleaseMutex(h->mutex);
}

http_sys_handle_t* http_event_new(void)
{
    http_sys_handle_t* h;

    h = (http_sys_handle_t*)http_malloc(sizeof(http_sys_handle_t));
    h->type = _HTTP_TYPE_EVENT;
    h->event= CreateEvent(NULL, FALSE, FALSE, NULL);

    return h;
}

void http_event_delete(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return;
    }

    CloseHandle(h->event);
    h->event = INVALID_HANDLE_VALUE;

    http_free(h);
}

http_bool_t http_event_trywait(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (WaitForSingleObject(h->event, 0) != WAIT_OBJECT_0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

http_bool_t http_event_wait(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (WaitForSingleObject(h->event, INFINITE) != WAIT_OBJECT_0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

http_bool_t http_event_waittimeout(http_sys_handle_t* h, int tmo)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return HTTP_FALSE;
    }

    if (WaitForSingleObject(h->event, tmo) != WAIT_OBJECT_0) {
        return HTTP_FALSE;
    }
    return HTTP_TRUE;
}

void http_event_set(http_sys_handle_t* h)
{
    if ((h == NULL) || (h->type != _HTTP_TYPE_EVENT)) {
        return;
    }
    SetEvent(h->event);
}

void http_msleep(int ms)
{
    Sleep(ms);
}

void http_begin_thread(http_thread_proc_t f, void* param)
{
    _beginthread(f, 0, param);
}

void http_end_thread(void)
{
    _endthread();
}

