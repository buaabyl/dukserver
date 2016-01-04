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
#include <ctype.h>
#include <time.h>

#include "libsocket.h"
#include "libhttp.h"
#include "duktape.h"
#include "dukpylib.h"

#include "wsgi_server.h"

static void* _duk_alloc(void *udata, duk_size_t size);
static void* _duk_realloc(void *udata, void *ptr, duk_size_t size);
static void _duk_free(void *udata, void *ptr);

static int _start_response(duk_context* ctx);

static void _put_request(duk_context* ctx, http_content_t* req);
static http_content_t* _get_response(duk_context* ctx);

static void _wsgi_initialize(void* param);
static void _wsgi_finalize(void* param);
static http_content_t* _wsgi_process(void* param, http_content_t* req);

void* _duk_alloc(void *udata, duk_size_t size)
{
    void* p;
    p = http_malloc(size);

    return p;
}

void* _duk_realloc(void *udata, void *ptr, duk_size_t size)
{
    void* p;

    p = http_realloc(ptr, size);

    return p;
}

void _duk_free(void *udata, void *ptr)
{
    http_free(ptr);
}

int _start_response(duk_context* ctx)
{
    duk_push_global_object(ctx);

    if (duk_get_top(ctx) > 0) {
        if (!duk_is_number(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "start_response(number, Array())!"); 
        }
        duk_dup(ctx, 0);
        duk_put_prop_string(ctx, -2, VARNAME_STATUS);
    }
    if (duk_get_top(ctx) > 1) {
        if (!duk_is_object(ctx, -1)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "start_response(number, Array())!");
        }

        duk_dup(ctx, 1);
        duk_put_prop_string(ctx, -2, VARNAME_HEADERS);
    }

    return 0;
}

void _put_request(duk_context* ctx, http_content_t* req)
{
    http_map_iter_t iter;
    http_map_t* m = NULL;
    const char* method = NULL;
    const char* content_type = NULL;
    void* pbuffer;
    time_t request_t;
    struct tm request_tm;
    char request_timestr[100];

    duk_push_global_object(ctx);
    duk_push_array(ctx);

    for (http_map_begin(req->dict, &iter);
         http_map_valid(&iter);
         http_map_next(&iter))
    {
        if (strcmp(iter.k, HTTP_METHOD) == 0) {
            duk_push_string(ctx, "REQUEST_METHOD");
            duk_push_string(ctx, iter.v);
            duk_put_prop(ctx, -3);

        } else if (strcmp(iter.k, HTTP_URL) == 0) {
            duk_push_string(ctx, "PATH_INFO");
            duk_push_string(ctx, iter.v);
            duk_put_prop(ctx, -3);

        } else if (strcmp(iter.k, HTTP_VERSION) == 0) {
            duk_push_string(ctx, "SERVER_PROTOCOL");
            duk_push_string(ctx, iter.v);
            duk_put_prop(ctx, -3);

        } else if (strcmp(iter.k, "REMOTE_PORT") == 0) {
            duk_push_string(ctx, iter.k);
            duk_push_int(ctx, atoi(iter.v));
            duk_put_prop(ctx, -3);

        } else {
            duk_push_string(ctx, iter.k);
            duk_push_string(ctx, iter.v);
            duk_put_prop(ctx, -3);
        }

    }

    time(&request_t);
    request_tm = *localtime(&request_t);

    strftime(request_timestr, sizeof(request_timestr), "%d %b %Y", &request_tm);
    duk_push_string(ctx, "wsgi.date");
    duk_push_string(ctx, request_timestr);
    duk_put_prop(ctx, -3);

    strftime(request_timestr, sizeof(request_timestr), "%H:%M:%S", &request_tm);
    duk_push_string(ctx, "wsgi.time");
    duk_push_string(ctx, request_timestr);
    duk_put_prop(ctx, -3);

    method = http_map_get(req->dict, HTTP_METHOD);
    content_type = http_map_get(req->dict, "Content-Type");
    if (method && content_type) {
        if (strcmp(method, "POST") == 0) {
            if (strcmp(content_type, HTTP_DEFAULT_FORM_ENCODED) == 0) {
                m = http_urldecode(req->body->ptr, req->body->size);
            }
        }
    }

    if (m) {
        duk_push_array(ctx);
        for (http_map_begin(m, &iter);
             http_map_valid(&iter);
             http_map_next(&iter))
        {
            duk_push_string(ctx, iter.k);
            duk_push_string(ctx, iter.v);
            duk_put_prop(ctx, -3);
        }
        duk_put_prop_string(ctx, -2, "wsgi.post");
        http_map_delete(m);

    } else if (req->body->size > 0) {
        duk_push_string(ctx, "wsgi.put");
        duk_push_fixed_buffer(ctx, req->body->size);
        pbuffer = duk_get_buffer(ctx, -1, NULL);
        memcpy(pbuffer, req->body->ptr, req->body->size);
        duk_put_prop(ctx, -3);
    }

    duk_put_prop_string(ctx, -2, "environ");
    duk_pop(ctx);//pop global
}

http_content_t* _get_response(duk_context* ctx)
{
    int status_code;
    http_content_t* res = NULL;

    int obj_id;
    const char* k;
    const char* v;
    const char* body_ptr = NULL;
    size_t body_size;

    duk_push_global_object(ctx);

    //get VARNAME_STATUS
    duk_get_prop_string(ctx, -1, VARNAME_STATUS);
    if (duk_is_undefined(ctx, -1)) {
        return NULL;
    }
    status_code = duk_to_int(ctx, -1);
    duk_pop(ctx);

    ////////////////////////////////////////////////////////
    res = http_content_new();
    http_start_response(res, status_code);

    //get VARNAME_HEADERS
    duk_get_prop_string(ctx, -1, VARNAME_HEADERS);
    if (!duk_is_object(ctx, -1)) {
        printf("forgot call start_response?\n");
        return NULL;
    }

    obj_id = duk_get_top(ctx) - 1;
    duk_enum(ctx, obj_id, DUK_ENUM_OWN_PROPERTIES_ONLY);
    while (duk_next(ctx, -1/*enumid*/, 1/*get value*/)) {
        if (!duk_is_string(ctx, -2)) {
            printf("TypeError: key of headers must be string!\n");
            duk_pop_2(ctx);
            continue;
        }
        if (!duk_is_string(ctx, -1)) {
            printf("TypeError: value of headers must be string!\n");
            duk_pop_2(ctx);
            continue;
        }

        k = duk_get_string(ctx, -2);
        v = duk_get_string(ctx, -1);
        http_header(res, k, v);
        duk_pop_2(ctx);
    }
    duk_pop(ctx);//pop enum 
    duk_pop(ctx);//pop headers
    duk_pop(ctx);//pop global

    if (duk_get_top(ctx) > 0) {
        body_ptr = duk_get_lstring(ctx, -1, &body_size);
        if ((body_ptr != NULL) && (body_size > 0)) {
            http_write(res, body_ptr, body_size);
        }
    }

    return res;
}

void _wsgi_initialize(void* param)
{
    wsgi_handle_t* h = (wsgi_handle_t*)param;
    wsgi_chain_t* p;
    int i;

    //h->ctx = duk_create_heap_default();
    h->ctx = duk_create_heap(_duk_alloc, _duk_realloc, _duk_free, NULL, NULL);
    dukopen_pylib(h->ctx);
    pylib_put_c_function(h->ctx, "start_response", _start_response);

    if (h->argc > 0) {
        pylib_put_args(h->ctx, h->argc, h->argv);

        duk_push_global_object(h->ctx);
        duk_push_string(h->ctx, h->argv[0]);
        duk_put_prop_string(h->ctx, -2, VARNAME_FILE);
        duk_pop(h->ctx);
    }

    p = h->init_chain;
    for (i = 0;i < h->nr_init;i++) {
        p->cb(h, p->param);
        p++;
    }

}

void _wsgi_finalize(void* param)
{
    wsgi_handle_t* h = (wsgi_handle_t*)param;
    wsgi_chain_t* p;
    int i;

    p = h->fini_chain + h->nr_fini-1;
    for (i = 0;i < h->nr_fini;i++) {
        p->cb(h, p->param);
        p--;
    }

    if (h->ctx != NULL) {
        duk_destroy_heap(h->ctx);
    }
}

http_content_t* _wsgi_process(void* param, http_content_t* req)
{
    int ok;
    wsgi_handle_t* h = (wsgi_handle_t*)param;
    http_content_t* res = NULL;

    _put_request(h->ctx, req);

    ok = pylib_eval_string(h->ctx, "include(" VARNAME_FILE ");");
    if (!ok) {
        duk_pop(h->ctx);
        return NULL;
    }

    ok = pylib_eval_string(h->ctx, "application(environ, start_response);");
    if (!ok) {
        duk_pop(h->ctx);
        return NULL;
    }

    res = _get_response(h->ctx);
    duk_pop(h->ctx);
    duk_gc(h->ctx, 0);

    return res;
}

wsgi_handle_t* wsgi_create(char* ip, int port)
{
    wsgi_handle_t* h;

    h = (wsgi_handle_t*)http_malloc(sizeof(wsgi_handle_t));
    memset(h, 0, sizeof(wsgi_handle_t));

    h->mpm = http_mpm_create(ip, port);
    if (h->mpm == NULL) {
        http_free(h);
        return NULL;
    }

    http_mpm_set_initialize(h->mpm, _wsgi_initialize, h);
    http_mpm_set_finalize(h->mpm, _wsgi_finalize);
    http_mpm_set_process(h->mpm, _wsgi_process);

    return h;
}

int wsgi_start(wsgi_handle_t* h, int argc, char* argv[], char* env[])
{
    int i = 0;
    char** args;

    if (argc <= 0) {
        return 0;
    }

    args = (char**)http_malloc(sizeof(char*) * argc);
    for (i = 0;i < argc;i++) {
        args[i] = http_strdup(argv[i]);
    }
    h->argc = argc;
    h->argv = args;

    return http_mpm_start(h->mpm);
}

void wsgi_close(wsgi_handle_t* h)
{
    int i;

    http_mpm_close(h->mpm);

    if (h->argv != NULL) {
        for (i = 0;i < h->argc;i++) {
            http_free(h->argv[i]);
        }
        http_free(h->argv);
    }

    if (h->nr_init > 0) {
        http_free(h->init_chain);
    }
    if (h->nr_fini > 0) {
        http_free(h->fini_chain);
    }

    http_free(h);
}

void wsgi_add_initialize_chain(wsgi_handle_t* h, wsgi_cb_t cb, void* param)
{
    wsgi_chain_t* p;
    if (h->nr_init == 0) {
        h->init_chain = (wsgi_chain_t*)http_malloc(sizeof(wsgi_chain_t));
        p = h->init_chain;
    } else {
        h->init_chain = (wsgi_chain_t*)http_realloc(
                h->init_chain,
                sizeof(wsgi_chain_t) * (h->nr_init + 1));
        p = &h->init_chain[h->nr_init];
    }
    h->nr_init++;

    p->cb = cb;
    p->param = param;
}

void wsgi_add_finalize_chain(wsgi_handle_t* h, wsgi_cb_t cb, void* param)
{
    wsgi_chain_t* p;
    if (h->nr_fini == 0) {
        h->fini_chain = (wsgi_chain_t*)http_malloc(sizeof(wsgi_chain_t));
        p = h->fini_chain;
    } else {
        h->fini_chain = (wsgi_chain_t*)http_realloc(
                h->fini_chain,
                sizeof(wsgi_chain_t) * (h->nr_fini + 1));
        p = &h->fini_chain[h->nr_fini];
    }
    h->nr_fini++;

    p->cb = cb;
    p->param = param;
}

