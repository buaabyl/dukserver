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
#include <time.h>

#include "libsocket.h"

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_connection.h"
#include "http_server.h"

httpd_server_t* httpd_create(const char* host, const char* ip, unsigned short port)
{
    httpd_server_t* srv;
    socklen_t srv_len;
    int err;

    srv = (httpd_server_t*)http_malloc(sizeof(httpd_server_t));
    if (host != NULL) {
        srv->host   = http_strdup(host);
    } else if (ip != NULL) {
        srv->host   = http_strdup(ip);
    } else {
        srv->host   = http_strdup("0.0.0.0");
    }
    srv->port   = port;
    srv->handle = (socket_t*)http_malloc(sizeof(socket_t));

    if (socket_create(srv->handle, AF_INET, SOCK_STREAM, IPPROTO_TCP) != IO_DONE) {
        http_free(srv->host);
        http_free(srv->handle);
        http_free(srv);
        return NULL;
    }

    if (ip == NULL) {
        srv->addr.sin_addr.s_addr  = htons(INADDR_ANY);
    } else {
        srv->addr.sin_addr.s_addr  = inet_addr(ip);
    }
    srv->addr.sin_family           = AF_INET;
    srv->addr.sin_port             = htons(port);
    srv_len = sizeof(srv->addr);

    err = socket_bind(srv->handle, (sockaddr*)&srv->addr, srv_len);
    if (err != IO_DONE) {
        printf("bind error = %d %s\n", err, socket_strerror(err));
        socket_destroy(srv->handle);
        http_free(srv->host);
        http_free(srv->handle);
        http_free(srv);
        return NULL;
    }
    
    err = socket_listen(srv->handle, 1);
    if (err != IO_DONE) {
        printf("listen error = %d %s\n", err, socket_strerror(err));
        socket_destroy(srv->handle);
        http_free(srv->host);
        http_free(srv->handle);
        http_free(srv);
        return NULL;
    }

    return srv;
}

void httpd_destroy(httpd_server_t* srv)
{
    http_free(srv->host);
    socket_destroy(srv->handle);
    http_free(srv->handle);
    http_free(srv);
}

http_connection_t* httpd_accept(httpd_server_t* srv)
{
    http_connection_t* conn;
    socklen_t cli_len;

    unsigned char* connected_ip;
    char remote_ip[16];
    int err;

    conn = (http_connection_t*)http_malloc(sizeof(http_connection_t));
    conn->host   = NULL;
    conn->handle = (socket_t*)http_malloc(sizeof(socket_t));

    cli_len = sizeof(conn->addr);
    err = socket_accept(srv->handle, conn->handle,
            (sockaddr*)&conn->addr, &cli_len, HTTP_ACCEPT_TIMEOUT);
    if (err != IO_DONE) {
        http_free(conn->host);
        http_free(conn->handle);
        http_free(conn);
        return NULL;
    }

    connected_ip = (unsigned char*)(&conn->addr.sin_addr.s_addr);
    sprintf(remote_ip, "%d.%d.%d.%d",
            connected_ip[0] & 0xFF,
            connected_ip[1] & 0xFF,
            connected_ip[2] & 0xFF,
            connected_ip[3] & 0xFF);
    conn->host = http_strdup(remote_ip);
    conn->port = ntohs(conn->addr.sin_port);

    return conn;
}

void httpd_close(http_connection_t* conn)
{
    http_free(conn->host);
    socket_shutdown(conn->handle, SD_BOTH);
    socket_destroy(conn->handle);
    http_free(conn->handle);
    http_free(conn);
}


