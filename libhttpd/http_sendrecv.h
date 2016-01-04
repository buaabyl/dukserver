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

#ifndef HTTP_SENDRECV_H_BEDF370F_8F30_11E5_A650_005056C00008_INCLUDED_
#define HTTP_SENDRECV_H_BEDF370F_8F30_11E5_A650_005056C00008_INCLUDED_

const char* http_get_status_message(int status);

int http_send_string(http_connection_t* conn, const char* s);
int http_send_nl(http_connection_t* conn);
int http_send_headers(http_connection_t* conn, http_map_t* hdr);
int http_send_binary(http_connection_t* conn, char* buffer, int buflen);
int http_send_file(http_connection_t* conn, const char* fn);

int http_recv_headers(http_connection_t* conn, http_content_t* ct);
int http_recv_binary(http_connection_t* conn, char* buffer, int expect_length);
int http_recv_file(http_connection_t* conn, FILE* fp, int expect_length);
int http_recv_body(http_connection_t* conn, http_content_t* ct);

//this two function define in libhttp.c
http_content_t* http_recv_request(http_connection_t* conn);
http_content_t* http_recv_response(http_connection_t* conn);


//@retval -1 error
//@retval 0  ok
//TODO:return http_bool_t
int http_start_response(http_content_t* ct, int status);
int http_start_request(http_content_t* ct, const char* method, const char* url);
int http_header(http_content_t* ct, const char* key, const char* value);
int http_write(http_content_t* ct, const char* buffer, int nr_bytes);
int http_bind_file(http_content_t* ct, const char* fn);

//@return number of bytes sent
int http_send_response(http_connection_t* conn, http_content_t* ct);
int http_send_request(http_connection_t* conn, http_content_t* ct);


#endif

