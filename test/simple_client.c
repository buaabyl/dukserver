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

int test_get(const char* host, const char* ip, int port)
{
    http_connection_t* conn;
    http_content_t* req;
    http_content_t* res;

    conn = http_connect(host, ip, port);
    if (conn == NULL) {
        return -1;
    }

    //send request
    req = http_content_new();
    http_start_request(req, "GET", "/index.html");
    http_header(req, "username", "fish");
    http_write(req, "hello", 5);
    http_send_request(conn, req);

    http_dump_content(req);
    http_content_delete(req);
    req = NULL;

    //receive response
    res = http_recv_response(conn);
    if (res) {
        http_dump_content(res);
        http_content_delete(res);
        res = NULL;
    }

    http_close(conn);

    return 0;
}

int test_post(const char* host, const char* ip, int port)
{
    http_connection_t* conn;
    http_content_t* req;
    http_content_t* res;
    http_map_t* m;
    char* form_buf;
    int form_len;

    m = http_map_new(100);
    http_map_set(m, "id", NULL);
    http_map_set(m, "name", "fish");
    http_map_set(m, "pass", "123");
    http_map_set(m, "refurl", "http://www.example.com/register.html?q=new");
    form_len = http_urlencode(m, &form_buf);

    conn = http_connect(host, ip, port);
    if (conn == NULL) {
        http_free(form_buf);
        http_map_delete(m);
        return -1;
    }

    //send request
    req = http_content_new();
    http_start_request(req, "POST", "/index.html");
    http_header(req, "Content-Type", "application/x-www-form-urlencoded");
    http_write(req, form_buf, form_len);
    http_send_request(conn, req);

    http_dump_content(req);
    http_content_delete(req);
    req = NULL;

    http_free(form_buf);
    http_map_delete(m);

    //receive response
    res = http_recv_response(conn);
    if (res) {
        http_dump_content(res);
        http_content_delete(res);
        res = NULL;
    }

    http_close(conn);

    return 0;
}

int main(int argc, char* argv[])
{
    socket_startup();
    http_init();

    //test_get("www.example.com", "127.0.0.1", 1000);
    test_post("www.example.com", "127.0.0.1", 1000);

    if (http_memory_usage() > 0) {
        printf("*memory leak:\n");
        http_memory_dump();
    }

    http_fini();
    socket_cleanup();

    return 0;
}

