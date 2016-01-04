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

#ifdef TARGET_WINDOWS
void start_thread(void (*f)(void*), void* param)
{
    _beginthread(f, 0, param);
}
#endif

int process(http_connection_t* conn)
{
    http_content_t* req;
    http_content_t* res;
    const char* method;
    const char* content_type;

    http_map_t* m;
    http_map_iter_t iter;

    //receive request
    req = http_recv_request(conn);
    if (req == NULL) {
        return -1;
    }
    http_dump_content(req);

    method = http_map_get(req->dict, HTTP_METHOD);
    content_type = http_map_get(req->dict, "Content-Type");
    if (method && content_type) {
        if (strcmp(method, "POST") == 0) {
            if (strcmp(content_type, HTTP_DEFAULT_FORM_ENCODED) == 0) {
                m = http_urldecode(req->body->ptr, req->body->size);
                if (m) {
                    for (http_map_begin(m, &iter);
                         http_map_valid(&iter);
                         http_map_next(&iter))
                    {
                        printf(" \"%s\" = \"%s\"\n", iter.k, iter.v);
                    }

                    http_map_delete(m);
                    m = NULL;
                }
            } else {
                printf("Error: not support form encoded \"%s\"\n", content_type);
            }
        }
    }

    http_content_delete(req);
    req = NULL;

    //send response
    res = http_content_new();
    http_start_response(res, 200);
    http_header(res, "username", "fish");
    http_write(res, "hello world!", 12);
    http_send_response(conn, res);
    http_content_delete(res);
    res = NULL;

    return 0;
}

void thread(void* param)
{
    http_connection_t* conn = (http_connection_t*)param;

    process(conn);

    httpd_close(conn);
    conn = NULL;
}

int server(void)
{
    httpd_server_t* srv;
    http_connection_t* conn;
    
    srv = httpd_create(NULL, "0.0.0.0", 1000);
    if (srv == NULL) {
        printf("Can't create httpd!\n");
        return -1;
    }

    while (1) {
        conn = httpd_accept(srv);
        if (conn == NULL) {
            break;
        }

        printf("*connected %s:%d\n", conn->host, conn->port);

#ifdef TARGET_WINDOWS
        start_thread(thread, conn);
#else
        thread(conn);
#endif

        printf("*disconnected\n");
    }

    httpd_destroy(srv);
    srv = NULL;

    return 0;
}

int main(int argc, char* argv[])
{
    socket_startup();
    http_init();

    printf("*memory %8d bytes\n", http_memory_usage());

    server();

    if (http_memory_usage() > 0) {
        printf("*memory leak:\n");
        http_memory_dump();
    }

    http_fini();
    socket_cleanup();

    return 0;
}



