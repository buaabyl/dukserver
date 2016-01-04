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

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_map.h"
#include "http_buffer.h"
#include "http_content.h"
#include "http_connection.h"
#include "http_server.h"
#include "http_sendrecv.h"
#include "http_parse.h"

#include "http_urlencode.h"
#include "http_urldecode.h"

#include "http_mpm.h"

struct http_mpm_handle_t
{
    ////////////////////////////////////
    //created by main thread 
    //delete by main thread 
    httpd_server_t*         srv;

    http_sys_handle_t*      event_break_accepter;
    http_sys_handle_t*      event_break_fastcgi;
    http_sys_handle_t*      event_accepter_exit;
    http_sys_handle_t*      event_fastcgi_exit;

    http_sys_handle_t*      mutex_fastcgi;
    http_sys_handle_t*      event_fire;
    http_sys_handle_t*      event_done;

    //created by main thread,
    //delete by frontend
    http_connection_t*      conn;

    ////////////////////////////////////
    //created by frontend
    //delete by frontend
    http_content_t**        req;

    //created by fastcgi
    //delete by frontend
    http_content_t**        res;

    ////////////////////////////////////
    //for fastcgi
    http_mpm_initialize_cb  initialize_cb;
    http_mpm_finalize_cb    finialize_cb;
    http_mpm_process_cb     process_cb;
    void* param;

};

static void _worker_fastcgi(void* param);
static void _worker_frontend(void* param);
static void _worker_accepter(void* param);


void _worker_fastcgi(void* param)
{
    http_mpm_handle_t* cfg;
    http_content_t* req = NULL;
    http_content_t* res = NULL;
    http_sys_handle_t* event_exit;
    int fastcgi_break = HTTP_FALSE;
    int nr_request = 0;

    cfg = (http_mpm_handle_t*)param;

    while (!fastcgi_break) {
        if (http_event_trywait(cfg->event_break_fastcgi)) {
            break;
        }

        cfg->initialize_cb(cfg->param);

        for (nr_request = 0;nr_request < HTTP_MPM_MAX_REQUESTS;nr_request++) {
            if (http_event_trywait(cfg->event_break_fastcgi)) {
                fastcgi_break = HTTP_TRUE;
                break;
            }

            if (!http_event_waittimeout(cfg->event_fire, HTTP_MPM_FASTCGI_TIMEOUT)) {
                continue;
            }

            req = *(cfg->req);
            res = cfg->process_cb(cfg->param, req);
            *(cfg->res) = res;

            http_event_set(cfg->event_done);
        }

        cfg->finialize_cb(cfg->param);
    }

    event_exit = cfg->event_fastcgi_exit;
    http_free(cfg);
    http_event_set(event_exit);

    http_end_thread();
}

void _worker_frontend(void* param)
{
    http_mpm_handle_t* cfg = NULL;
    http_content_t* req = NULL;
    http_content_t* res = NULL;
    char remote_port[10];
    const char* version;
    http_bool_t fastcgi_break = HTTP_FALSE;

    cfg = (http_mpm_handle_t*)param;

    while(!fastcgi_break) {
        //receive one request.
        req = http_recv_request(cfg->conn);
        if (req == NULL) {
            break;
        }

        //prepare connection information
        http_map_set(req->dict, "REMOTE_ADDR", cfg->conn->host);

        sprintf(remote_port, "%d", cfg->conn->port);
        http_map_set(req->dict, "REMOTE_PORT", remote_port);

        //send to fastcgi thread
        if (http_mutex_take(cfg->mutex_fastcgi)) {
            *(cfg->req) = req;
            http_event_set(cfg->event_fire);

            if (http_event_wait(cfg->event_done)) {
                res = *(cfg->res);
                *(cfg->res) = NULL;
            }

            http_mutex_give(cfg->mutex_fastcgi);

        } else {
            printf("_worker_frontend mutex timeout!\n");
        }

        if (res == NULL) {
            res = http_content_new();
            http_start_response(res, 500);
            http_write(res, "Fastcgi Error", 13);
            fastcgi_break = HTTP_TRUE;
        }

        //send response
        http_send_response(cfg->conn, res);
        http_content_delete(res);
        res = NULL;

        //check http version
        version = http_map_get(req->dict, HTTP_VERSION);
        if (version && (strcmp(version, "HTTP/1.0") == 0)) {
            fastcgi_break = HTTP_TRUE;
        }
        http_content_delete(req);
        req = NULL;
    }

    httpd_close(cfg->conn);
    cfg->conn = NULL;

    http_free(cfg);

    http_end_thread();
}

void _worker_accepter(void* param)
{
    http_mpm_handle_t* oldcfg = NULL;
    http_mpm_handle_t* cfg = NULL;
    http_connection_t* conn = NULL;
    http_content_t* interwork_req   = NULL;
    http_content_t* interwork_res   = NULL;
    http_sys_handle_t* event_exit;

    oldcfg = (http_mpm_handle_t*)param;

    cfg = (http_mpm_handle_t*)http_malloc(sizeof(http_mpm_handle_t));
    cfg->srv                    = NULL;
    cfg->event_break_accepter   = NULL;
    cfg->event_break_fastcgi    = oldcfg->event_break_fastcgi;
    cfg->event_accepter_exit    = NULL;
    cfg->event_fastcgi_exit     = oldcfg->event_fastcgi_exit;

    cfg->mutex_fastcgi          = NULL;
    cfg->event_fire             = oldcfg->event_fire;
    cfg->event_done             = oldcfg->event_done;
    cfg->conn                   = NULL;
    cfg->req                    = &interwork_req;
    cfg->res                    = &interwork_res;
    cfg->initialize_cb          = oldcfg->initialize_cb;
    cfg->finialize_cb           = oldcfg->finialize_cb;
    cfg->process_cb             = oldcfg->process_cb;
    cfg->param                  = oldcfg->param;
    http_begin_thread(_worker_fastcgi, cfg);

    while (1) {
        if (http_event_trywait(oldcfg->event_break_accepter)) {
            break;
        }

        conn = httpd_accept(oldcfg->srv);
        if (conn == NULL) {
            continue;
        }

        cfg = (http_mpm_handle_t*)http_malloc(sizeof(http_mpm_handle_t));
        cfg->event_break_accepter   = NULL;
        cfg->event_break_fastcgi    = NULL;
        cfg->event_accepter_exit    = NULL;
        cfg->event_fastcgi_exit     = NULL;

        cfg->mutex_fastcgi          = oldcfg->mutex_fastcgi;
        cfg->event_fire             = oldcfg->event_fire;
        cfg->event_done             = oldcfg->event_done;
        cfg->conn                   = conn;
        cfg->req                    = &interwork_req;
        cfg->res                    = &interwork_res;
        cfg->initialize_cb          = NULL;
        cfg->finialize_cb           = NULL;
        cfg->process_cb             = NULL;
        cfg->param                  = NULL;
        http_begin_thread(_worker_frontend, cfg);
    }

    event_exit = oldcfg->event_accepter_exit;
    http_free(oldcfg);
    http_event_set(event_exit);

    http_end_thread();
}

http_mpm_handle_t* http_mpm_create(char* ip, int port)
{
    http_mpm_handle_t* h = NULL;
    httpd_server_t* srv = NULL;

    srv = httpd_create(NULL, ip, port);
    if (srv == NULL) {
        printf("Can't create httpd!\n");
        return NULL;
    }

    h = (http_mpm_handle_t*)http_malloc(sizeof(http_mpm_handle_t));

    h->srv                  = srv;
    h->event_break_accepter = http_event_new();
    h->event_break_fastcgi  = http_event_new();
    h->event_accepter_exit  = http_event_new();
    h->event_fastcgi_exit   = http_event_new();

    h->mutex_fastcgi        = http_mutex_new();
    h->event_fire           = http_event_new();
    h->event_done           = http_event_new();

    h->conn                 = NULL;
    h->req                  = NULL;
    h->res                  = NULL;

    h->initialize_cb        = NULL;
    h->finialize_cb         = NULL;
    h->process_cb           = NULL;
    h->param                = NULL;

    return h;
}

void http_mpm_set_initialize(
        http_mpm_handle_t* h,
        http_mpm_initialize_cb cb,
        void* initparam)
{
    h->initialize_cb = cb;
    h->param= initparam;
}

void http_mpm_set_finalize(
        http_mpm_handle_t* h,
        http_mpm_finalize_cb cb)
{
    h->finialize_cb = cb;
}

void http_mpm_set_process(
        http_mpm_handle_t* h,
        http_mpm_process_cb cb)
{
    h->process_cb = cb;
}

http_bool_t http_mpm_start(http_mpm_handle_t* h)
{
    http_mpm_handle_t* cfg = NULL;

    if (!h || (!h->process_cb)) {
        return HTTP_FALSE;
    }

    cfg = (http_mpm_handle_t*)http_malloc(sizeof(http_mpm_handle_t));
    cfg->srv                    = h->srv;
    cfg->event_break_accepter   = h->event_break_accepter;
    cfg->event_break_fastcgi    = h->event_break_fastcgi;
    cfg->event_accepter_exit    = h->event_accepter_exit;
    cfg->event_fastcgi_exit     = h->event_fastcgi_exit;

    cfg->mutex_fastcgi          = h->mutex_fastcgi;
    cfg->event_fire             = h->event_fire;
    cfg->event_done             = h->event_done;

    cfg->conn                   = NULL;
    cfg->req                    = NULL;
    cfg->res                    = NULL;

    cfg->initialize_cb          = h->initialize_cb;
    cfg->finialize_cb           = h->finialize_cb;
    cfg->process_cb             = h->process_cb;
    cfg->param                  = h->param;

    http_begin_thread(_worker_accepter, cfg);

    return HTTP_TRUE;
}

void http_mpm_close(http_mpm_handle_t* p)
{
    http_event_set(p->event_break_accepter);
    http_event_set(p->event_break_fastcgi);

    http_event_wait(p->event_accepter_exit);
    http_event_wait(p->event_fastcgi_exit);

    http_event_delete(p->event_break_accepter);
    http_event_delete(p->event_break_fastcgi);
    http_event_delete(p->event_accepter_exit);
    http_event_delete(p->event_fastcgi_exit);
    http_mutex_delete(p->mutex_fastcgi);
    http_event_delete(p->event_fire);
    http_event_delete(p->event_done);

    httpd_destroy(p->srv);

    http_free(p);
}

