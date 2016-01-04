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
#include "http_connection.h"
#include "http_client.h"

http_connection_t* http_connect(const char* host, const char* ip, unsigned short port)
{
    http_connection_t* conn;
    socklen_t cli_len;
    int err;

    if (ip == NULL) {
        return NULL;
    }

    conn = (http_connection_t*)http_malloc(sizeof(http_connection_t));
    conn->port = port;
    if (host != NULL) {
        conn->host = http_strdup(host);
    } else {
        conn->host = http_strdup(ip);
    }
    conn->handle = (socket_t*)http_malloc(sizeof(socket_t));

    if (socket_create(conn->handle, AF_INET, SOCK_STREAM, IPPROTO_TCP) != IO_DONE) {
        http_free(conn->handle);
        http_free(conn);
        return NULL;
    }

    conn->addr.sin_addr.s_addr  = inet_addr(ip);
    conn->addr.sin_family       = AF_INET;
    conn->addr.sin_port         = htons(port);
    cli_len = sizeof(conn->addr);

    err = socket_connect(conn->handle,
            (sockaddr*)&conn->addr, cli_len, 2);
    if (err != IO_DONE) {
        printf("Socket Error %d\n", err);
        socket_destroy(conn->handle);
        http_free(conn->host);
        http_free(conn->handle);
        http_free(conn);
        return NULL;
    }

    return conn;
}

void http_close(http_connection_t* conn)
{
    socket_shutdown(conn->handle, SD_BOTH);
    http_free(conn->host);
    socket_destroy(conn->handle);
    http_free(conn->handle);
    http_free(conn);
}

