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

#ifndef WSGI_SERVER_H_61A9B6B0_AAAB_11E5_93C6_005056C00008_INCLUDED_
#define WSGI_SERVER_H_61A9B6B0_AAAB_11E5_93C6_005056C00008_INCLUDED_

#define VARNAME_FILE     "__file__"
#define VARNAME_STATUS   "__http_response_status__"
#define VARNAME_HEADERS  "__http_response_headers__"


typedef struct wsgi_chain_t wsgi_chain_t;
typedef struct wsgi_handle_t wsgi_handle_t;
typedef void (*wsgi_cb_t)(wsgi_handle_t* srv, void* param);

struct wsgi_chain_t {
    wsgi_cb_t           cb;
    void*               param;
};

struct wsgi_handle_t {
    http_mpm_handle_t*  mpm;
    duk_context*        ctx;
    int                 argc;
    char**              argv;
    int                 nr_init;
    int                 nr_fini;
    wsgi_chain_t*       init_chain;
    wsgi_chain_t*       fini_chain;
};

wsgi_handle_t* wsgi_create(char* ip, int port);
int wsgi_start(wsgi_handle_t* h, int argc, char* argv[], char* env[]);
void wsgi_close(wsgi_handle_t* h);

void wsgi_add_initialize_chain(wsgi_handle_t* h, wsgi_cb_t cb, void* param);
void wsgi_add_finalize_chain(wsgi_handle_t* h, wsgi_cb_t cb, void* param);

#endif

