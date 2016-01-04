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

#ifndef HTTP_SYS_H_2DD3B70F_8E6C_11E5_A502_005056C00008_INCLUDED_
#define HTTP_SYS_H_2DD3B70F_8E6C_11E5_A502_005056C00008_INCLUDED_

////////////////////////////////////////////////////////////
typedef enum {
    HTTP_FALSE = 0,
    HTTP_TRUE  = 1
}http_bool_t;

void http_init(void);
void http_fini(void);

void http_msleep(int ms);


////////////////////////////////////////////////////////////
int     http_memory_usage(void);
void    http_memory_dump(void);

#define http_malloc(size)       http_malloc_((size), __FILE__, __LINE__)
#define http_realloc(p, size)   http_realloc_((p), (size), __FILE__, __LINE__)
#define http_free(p)            http_free_((p), __FILE__, __LINE__)

void*   http_malloc_(int size, char* file, int line);
void*   http_realloc_(void* p, int size, char* file, int line);
void    http_free_(void* p, char* file, int line);


////////////////////////////////////////////////////////////
typedef struct http_sys_handle_t http_sys_handle_t;

http_sys_handle_t*  http_mutex_new(void);
void                http_mutex_delete(http_sys_handle_t* h);
http_bool_t         http_mutex_take(http_sys_handle_t* h);
void                http_mutex_give(http_sys_handle_t* h);

http_sys_handle_t*  http_event_new(void);
void                http_event_delete(http_sys_handle_t* h);
http_bool_t         http_event_trywait(http_sys_handle_t* h);
http_bool_t         http_event_wait(http_sys_handle_t* h);
http_bool_t         http_event_waittimeout(http_sys_handle_t* h, int tmo);
void                http_event_set(http_sys_handle_t* h);

////////////////////////////////////////////////////////////
typedef void (http_thread_proc_t)(void* param);

void http_begin_thread(http_thread_proc_t f, void* param);
void http_end_thread(void);



#endif

