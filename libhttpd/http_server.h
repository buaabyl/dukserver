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

#ifndef HTTP_SERVER_H_4D470970_5DD0_11E5_963F_005056C00008_INCLUDED_
#define HTTP_SERVER_H_4D470970_5DD0_11E5_963F_005056C00008_INCLUDED_

////////////////////////////////////////////////////////////
typedef struct _httpd_server_t {
    char*               host;
    int                 port;
    socket_t*           handle;
    struct sockaddr_in  addr;
}httpd_server_t;

////////////////////////////////////////////////////////////
//@param host   NULL or "www.example.com"
//@param ip     NULL or "127.0.0.1"
httpd_server_t* httpd_create(const char* host, const char* ip, unsigned short port);
void httpd_destroy(httpd_server_t* srv);

http_connection_t* httpd_accept(httpd_server_t* srv);
void httpd_close(http_connection_t* conn);

#endif

