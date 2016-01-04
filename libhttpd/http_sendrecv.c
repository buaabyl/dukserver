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
#include "http_parse.h"

////////////////////////////////////////////////////////
typedef struct {
    int status;
    const char* short_msg;
    const char* long_msg;
}_http_status_message_t;


////////////////////////////////////////////////////////
static int _preprocess_request(http_connection_t* conn, http_content_t* ct);
static int _get_file_size(const char* fn);

//@ref copy from python/lib/BaseHttpServer.py
_http_status_message_t _status_msg_map[] = {
    {100, "Continue",
        "Request received, please continue"},
    {101, "Switching Protocols",
        "Switching to new protocol; obey Upgrade header"},


    {200, "OK",
        "Request fulfilled, document follows"},
    {201, "Created",
        "Document created, URL follows"},
    {202, "Accepted",
        "Request accepted, processing continues off-line"},
    {203, "Non-Authoritative Information",
        "Request fulfilled from cache"},
    {204, "No Content",
        "Request fulfilled, nothing follows"},
    {205, "Reset Content",
        "Clear input form for further input."},
    {206, "Partial Content",
        "Partial content follows."},


    {300, "Multiple Choices",
        "Object has several resources -- see URI list"},
    {301, "Moved Permanently",
        "Object moved permanently -- see URI list"},
    {302, "Found",
        "Object moved temporarily -- see URI list"},
    {303, "See Other",
        "Object moved -- see Method and URL list"},
    {304, "Not Modified",
        "Document has not changed since given time"},
    {305, "Use Proxy",
        "You must use proxy specified in Location to access this resource."},
    {307, "Temporary Redirect",
        "Object moved temporarily -- see URI list"},


    {400, "Bad Request",
        "Bad request syntax or unsupported method"},
    {401, "Unauthorized",
        "No permission -- see authorization schemes"},
    {402, "Payment Required",
        "No payment -- see charging schemes"},
    {403, "Forbidden",
        "Request forbidden -- authorization will not help"},
    {404, "Not Found",
        "Nothing matches the given URI"},
    {405, "Method Not Allowed",
        "Specified method is invalid for this resource."},
    {406, "Not Acceptable",
        "URI not available in preferred format."},
    {407, "Proxy Authentication Required",
        "You must authenticate with this proxy before proceeding."},
    {408, "Request Timeout",
        "Request timed out; try again later."},
    {409, "Conflict",
        "Request conflict."},
    {410, "Gone",
        "URI no longer exists and has been permanently removed."},
    {411, "Length Required",
        "Client must specify Content-Length."},
    {412, "Precondition Failed",
        "Precondition in headers is false."},
    {413, "Request Entity Too Large",
        "Entity is too large."},
    {414, "Request-URI Too Long",
        "URI is too long."},
    {415, "Unsupported Media Type",
        "Entity body in unsupported format."},
    {416, "Requested Range Not Satisfiable",
        "Cannot satisfy request range."},
    {417, "Expectation Failed",
        "Expect condition could not be satisfied."},


    {500, "Internal Server Error",
        "Server got itself in trouble"},
    {501, "Not Implemented",
        "Server does not support this operation"},
    {502, "Bad Gateway",
        "Invalid responses from another server/proxy."},
    {503, "Service Unavailable",
        "The server cannot process the request due to a high load"},
    {504, "Gateway Timeout",
        "The gateway server did not receive a timely response"},
    {505, "HTTP Version Not Supported", "Cannot fulfill request."},
};

////////////////////////////////////////////////////////
const char* http_get_status_message(int status)
{
    int i;
    int n;

    n = sizeof(_status_msg_map)/sizeof(_status_msg_map[0]);
    for (i = 0;i < n;i++) {
        if (_status_msg_map[i].status == status) {
            break;
        }
    }

    if (i == n) {
        status = 500;
        for (i = 0;i < n;i++) {
            if (_status_msg_map[i].status == status) {
                break;
            }
        }
    }

    if (i == n) {
        i = 0;
    }

    return _status_msg_map[i].short_msg;
}

int http_send_string(http_connection_t* conn, const char* s)
{
    size_t sent;

    socket_send(conn->handle, s, strlen(s), &sent, HTTP_SEND_TIMEOUT);

    return sent;
}

int http_send_nl(http_connection_t* conn)
{
    char eol[3] = "\r\n";
    size_t sent;

    socket_send(conn->handle, eol, 2, &sent, HTTP_SEND_TIMEOUT);

    return sent;
}

int http_send_headers(http_connection_t* conn, http_map_t* hdr)
{
    http_map_iter_t iter;
    char* buffer;
    char* p;
    int key_size;
    int val_size;
    size_t sent;
    size_t total_sent = 0;

    for (http_map_begin(hdr, &iter);
        http_map_valid(&iter);
        http_map_next(&iter))
    {
        if ((iter.k == NULL) || (iter.v == NULL)) {
            continue;
        }

        //skip internal header
        //such like " VERSION", etc.
        if (iter.k[0] == ' ') {
            continue;
        }

        key_size = strlen(iter.k);
        if (iter.v) {
            val_size = strlen(iter.v);
        } else {
            val_size = 0;
        }
        buffer = (char*)malloc(key_size + 2 + val_size + 3);
        memset(buffer, 0, key_size + 2 + val_size + 3);

        p = buffer;
        memcpy(p, iter.k, key_size);
        p += key_size;
        *p++ = ':';
        *p++ = ' ';
        if (val_size > 0) {
            memcpy(p, iter.v, val_size);
            p += val_size;
        }
        *p++ = '\r';
        *p++ = '\n';

        socket_send(conn->handle, buffer, p - buffer, &sent, HTTP_SEND_TIMEOUT);
        free(buffer);
        buffer = NULL;

        total_sent += sent;
    }

    return total_sent;
}

int http_send_binary(http_connection_t* conn, char* buffer, int buflen)
{
    char* p;
    char* pend;
    int to_send;
    size_t sent;

    p = buffer;
    pend = p + buflen;
    while (p < pend) {
        if (pend - p > HTTP_MAX_SEND_BLOCK) {
            to_send = HTTP_MAX_SEND_BLOCK;
        } else {
            to_send = pend - p;
        }

        socket_send(conn->handle, p, to_send, &sent, HTTP_SEND_TIMEOUT);
        p += to_send;
    }

    return buflen;
}

int http_send_file(http_connection_t* conn, const char* fn)
{
    FILE* fp;
    int total_bytes = 0;
    int len;
    char* buffer;
    size_t sent;

    if ((fp = fopen(fn, "rb")) == NULL) {
        return 0;
    }

    buffer = (char*)http_malloc(HTTP_MAX_SEND_BLOCK);
    while (!feof(fp)) {
        len = fread(buffer, 1, HTTP_MAX_SEND_BLOCK, fp);
        if (len <= 0) {
            break;
        }

        printf("read %d bytes\n", len);
        socket_send(conn->handle, buffer, len, &sent, HTTP_SEND_TIMEOUT);
        total_bytes += sent;

        printf("sent %d bytes\n", sent);
    }
    http_free(buffer);

    fclose(fp);
    fp = NULL;

    return total_bytes;
}

int http_recv_headers(http_connection_t* conn, http_content_t* ct)
{
    char* pbuf;
    const char* pempty_nl;

    size_t to_received;
    size_t got;
    int nr_received;

    int start_pos;

    nr_received = 0;
    while (1) {
        got         = 0;
        pbuf        = ct->headerbuf->buf + nr_received;
        to_received = ct->headerbuf->maxsize - nr_received;

        socket_recv(conn->handle, pbuf, to_received, &got, HTTP_RECV_TIMEOUT);
        if (got <= 0) {
            break;
        }
        nr_received += got;

        //need to check previous received bytes?
        start_pos = 0;
        if (nr_received > (int)got) {
            start_pos = -3;
        }

        //"\r\n\r\n"
        pempty_nl = http_find_crlf(&pbuf[start_pos], (got-3)-start_pos);
        if (pempty_nl == NULL) {
            continue;
        }

        //"\r\n\r\n"
        // ^
        // | pempty_nl
        //
        ct->header->size = &pempty_nl[4] - ct->headerbuf->buf;
        ct->body->size   = nr_received - ct->header->size;

        ct->header->ptr = ct->headerbuf->buf;
        if (ct->body->size == 0) {
            ct->body->ptr = NULL;
        } else {
            ct->body->ptr = ct->headerbuf->buf + ct->header->size;
        }

        return ct->header->size;
    }

    return -1;
}

int http_recv_binary(http_connection_t* conn, char* buffer, int expect_length)
{
    char* pbuf;
    size_t to_received;
    size_t nr_received;
    size_t got;

    nr_received = 0;

    //printf("* to_received = %d\n", expect_length);

    while (nr_received < (size_t)expect_length) {
        pbuf        = buffer + nr_received;
        to_received = expect_length - nr_received;

        socket_recv(conn->handle, pbuf, to_received, &got, HTTP_RECV_TIMEOUT);
        if (got <= 0) {
            break;
        }

        nr_received += got;
    }

    return nr_received;
}

int http_recv_file(http_connection_t* conn, FILE* fp, int expect_length)
{
    char buf[K_BUFFER_1KB];
    size_t to_received;
    size_t nr_received;
    size_t got;

    nr_received = 0;

    while (nr_received < (size_t)expect_length) {
        to_received = expect_length - nr_received;
        if (to_received > sizeof(buf)) {
            to_received = sizeof(buf);
        }

        socket_recv(conn->handle, buf, to_received, &got, HTTP_RECV_TIMEOUT);
        if (got <= 0) {
            break;
        }
        
        fwrite(buf, 1, got, fp);
        nr_received += got;
    }

    return nr_received;
}

int http_recv_body(http_connection_t* conn, http_content_t* ct)
{
    char tmpfilename[1024];
    const char* content_length_str;
    int content_length;
    int nr_received;
    size_t to_received;
    char* pbuf;
    int redirect_to_file = 0;
    FILE* fp = NULL;;

    content_length_str = http_map_get(ct->dict, "Content-Length");
    if (content_length_str == NULL) {
        return 0;
    }

    content_length = atoi(content_length_str);
    //printf("* content_length = %d\n", content_length);
    if (content_length <= 0) {
        return 0;
    }

    if (ct->body->size >= content_length) {
        //printf("* content_length (reset) = %d\n", content_length);
        ct->body->size = content_length;
        return 0;
    }

    to_received = content_length - ct->body->size;
    if (content_length > HTTP_MAX_BUFFER_SIZE) {
        redirect_to_file = 1;
    }

    //prepare receive more bytes
    if (redirect_to_file) {
        //printf("* content_length (file) = %d\n", content_length);

        sprintf(tmpfilename, "./%04x-%08x-%08x.tmp",
                rand() & 0xFFFFu, (int)time(NULL), (int)clock());
        //printf("* payload too big, redirect to file \"%s\"\n",
        //        tmpfilename);

        http_map_set(ct->dict, HTTP_TMPFILE, tmpfilename);

        fp = fopen(tmpfilename, "wb");
        if (fp == NULL) {
            return -1;
        }

    } else {
        //printf("* content_length (ram) = %d\n", content_length);
        pbuf = ct->bodybuf->buf;
    }

    //copy received data bytes.
    if (ct->body->size > 0) {
        if (redirect_to_file) {
            fwrite(ct->body->ptr, 1, ct->body->size, fp);
        } else {
            memcpy(pbuf, ct->body->ptr, ct->body->size);
            ct->bodybuf->size = ct->body->size;
            pbuf += ct->body->size;
        }
    }

    //begin tcp receiving
    if (redirect_to_file) {
        ct->body->ptr = NULL;
        nr_received = http_recv_file(conn, fp, to_received);
        fclose(fp);
        fp = NULL;
    } else {
        ct->body->ptr = ct->bodybuf->buf;
        nr_received = http_recv_binary(conn, pbuf, to_received);
        ct->bodybuf->size += nr_received;
    }

    ct->body->size = content_length;

    return nr_received;
}

http_content_t* http_recv_request(http_connection_t* conn)
{
    int got;
    http_bool_t done;
    http_content_t* ct;
    const char* expect_str;

    ct = http_content_new();
    got = http_recv_headers(conn, ct);
    if (got <= 0) {
        http_content_delete(ct);
        return NULL;
    }

    done = http_parse_request_headers(ct->header->ptr, ct->header->size, ct->dict);
    if (!done) {
        http_content_delete(ct);
        return NULL;
    }

    expect_str = http_map_get(ct->dict, "Expect");
    if (expect_str && (strcmp(expect_str, "100-continue") == 0)) {
        http_send_string(conn, HTTP_DEFAULT_RESPONSE_EXPECT100);
    }

    http_recv_body(conn, ct);

    return ct;
}

http_content_t* http_recv_response(http_connection_t* conn)
{
    size_t got;
    http_bool_t done;
    http_content_t* ct;

    ct = http_content_new();
    got = http_recv_headers(conn, ct);
    if (got <= 0) {
        http_content_delete(ct);
        return NULL;
    }

    done = http_parse_response_headers(ct->header->ptr, ct->header->size, ct->dict);
    if (!done) {
        http_content_delete(ct);
        return NULL;
    }

    http_recv_body(conn, ct);

    return ct;
}

int http_start_response(http_content_t* ct, int status)
{
    char str[100];
    time_t t;
    struct tm tm;

    http_map_set(ct->dict, HTTP_VERSION, "HTTP/1.1");

    sprintf(str, "%d", status);
    http_map_set(ct->dict, HTTP_STATUS, str);

    //"Fri, 18 Sep 2015 07:17:31 GMT"
    time(&t);
    tm = *localtime(&t);
    strftime(str, sizeof(str), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    http_map_set(ct->dict, "Date", str);

    http_map_set(ct->dict, "Server", HTTP_DEFAULT_SERVER);
    http_map_set(ct->dict, "Content-Type", HTTP_DEFAULT_CONTENT_TYPE);

    return 0;
}

int http_start_request(http_content_t* ct, const char* method, const char* url)
{
    if ((method == NULL) || (url == NULL)) {
        return -1;
    }

    http_map_set(ct->dict, HTTP_METHOD, method);
    http_map_set(ct->dict, HTTP_URL,    url);
    http_map_set(ct->dict, HTTP_VERSION,"HTTP/1.1");
    http_map_set(ct->dict, "User-Agent",  "libhttp/1.0.0");
    http_map_set(ct->dict, "Accept",      "*/*");
    http_map_set(ct->dict, "Host",        "127.0.0.1");

    ct->body->ptr = ct->bodybuf->buf;
    ct->body->size= 0;

    return 0;
}

int http_header(http_content_t* ct, const char* key, const char* value)
{
    if (key == NULL) {
        return -1;
    }

    http_map_set(ct->dict, key, value);

    return 0;
}

int http_write(http_content_t* ct, const char* buffer, int nr_bytes)
{
    int new_size;

    if ((buffer == NULL) || (nr_bytes <= 0)) {
        return -1;
    }

    if (ct->bodybuf->size + nr_bytes > ct->bodybuf->maxsize) {
        new_size = ct->bodybuf->size + nr_bytes;
        //printf("realloc %d -> %d\n", ct->bodybuf->maxsize, new_size);
        ct->bodybuf->buf     = (char*)http_realloc(ct->bodybuf->buf, new_size);
        ct->bodybuf->maxsize = new_size;
    }

    memcpy(ct->bodybuf->buf + ct->bodybuf->size, buffer, nr_bytes);
    ct->bodybuf->size += nr_bytes;
    ct->body->ptr  = ct->bodybuf->buf;
    ct->body->size = ct->bodybuf->size;

    return 1;
}

int http_bind_file(http_content_t* ct, const char* fn)
{
    http_map_set(ct->dict, HTTP_SENDFILE, fn);

    return 1;
}

int http_send_response(http_connection_t* conn, http_content_t* ct)
{
    const char* status_code_str;
    const char* status_msg_str;
    const char* version_str;
    int status_code;

    http_map_iter_t iter;

    int nr_bytes;
    char* buffer;
    char* p;

    char str[100];
    size_t sent;

    status_code_str = http_map_get(ct->dict, HTTP_STATUS);
    version_str     = http_map_get(ct->dict, HTTP_VERSION);
    if ((status_code_str == NULL) || (version_str == NULL)) {
        return -1;
    }

    status_code     = atoi(status_code_str);
    status_msg_str  = http_get_status_message(status_code);
    if (status_msg_str == NULL) {
        return -1;
    }

    if ((status_code == 200) || (status_code == 404)) {
        if (ct->body->size <= 0) {
            http_write(ct, "None", 4);
        }
    }

    if (ct->body->size > 0) {
        sprintf(str, "%d", ct->body->size);
        http_map_set(ct->dict, "Content-Length", str);
    }

    ////////////////////////////////////////////////////////
    //build response line
    buffer = (char*)http_malloc(HTTP_MAX_SEND_BLOCK);
    p = buffer;
    nr_bytes = sprintf(p, "%s %s %s\r\n",
            version_str, status_code_str, status_msg_str);
    p += nr_bytes; 


    ////////////////////////////////////////////////////////
    //build headers
    for (http_map_begin(ct->dict, &iter);
        http_map_valid(&iter);
        http_map_next(&iter))
    {
        if ((iter.k == NULL) || (iter.v == NULL)) {
            continue;
        }

        //skip internal header
        //such like " VERSION", etc.
        if (iter.k[0] == ' ') {
            continue;
        }

        if (iter.v) {
            nr_bytes = sprintf(p, "%s: %s\r\n", iter.k, iter.v);
        } else {
            nr_bytes = sprintf(p, "%s:\r\n", iter.k);
        }
        p += nr_bytes;
    }

    *p++ = '\r';
    *p++ = '\n';

    ////////////////////////////////////////////////////////
    //send to remote
    socket_send(conn->handle, buffer, p - buffer, &sent, HTTP_SEND_TIMEOUT);

    if (ct->body->size > 0) {
        sent += http_send_binary(conn, ct->body->ptr, ct->body->size);
    }

    http_free(buffer);
    buffer = NULL;

    return sent;
}

int _preprocess_request(http_connection_t* conn, http_content_t* ct)
{
    unsigned char* connected_ip;
    const char* url_str;

    int url_n;
    const char* pslash;//   '/'
    const char* pcolon;//   ':'

    char* request_host;
    char* request_path;

    ////////////////////////////////////////////////////////
    connected_ip= (unsigned char*)(&conn->addr.sin_addr.s_addr);
    url_str     = http_map_get(ct->dict, HTTP_URL);

    //check and build 'Host' and 'Path'
    url_n  = strlen(url_str);
    pslash = http_find_char(url_str, '/', url_n);
    pcolon = http_find_char(url_str, ':', url_n);

    if (pslash != NULL) {
        if (pslash == url_str) {//format: /...
            request_host = NULL;

        } else if (pcolon < pslash) {//format: 127.0.0.1:1000/...
            request_host = http_strndup(url_str, pcolon - url_str);

        } else {//format: 127.0.0.1/...
            request_host = http_strndup(url_str, pslash - url_str);
        }

        request_path = http_strdup(pslash);

    } else {//format: 127.0.0.1
        request_host = NULL;
        request_path = http_strdup("/");
    }

    //build Host from connection
    if (request_host == NULL) {
        if (conn->host != NULL) {
            request_host = http_strdup(conn->host);
        } else {
            request_host = http_malloc(sizeof("255.255.255.255"));
            sprintf(request_host, "%d.%d.%d.%d",
                    connected_ip[0] & 0xFF,
                    connected_ip[1] & 0xFF,
                    connected_ip[2] & 0xFF,
                    connected_ip[3] & 0xFF);
        }
    }

    http_map_set(ct->dict, HTTP_URL, request_path);
    http_free(request_path);
    request_path = NULL;

    http_map_set(ct->dict, "Host", request_host);
    http_free(request_host);
    request_host = NULL;

    return 0;
}

int _get_file_size(const char* fn)
{
    FILE* fp;
    int len;

    if ((fp = fopen(fn, "rb")) == NULL) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);

    fclose(fp);
    fp = NULL;

    return len;
}

int http_send_request(http_connection_t* conn, http_content_t* ct)
{
    const char* method_str;
    const char* url_str;
    const char* version_str;
    const char* sendfile_str;
    int content_length;

    http_map_iter_t iter;

    int nr_bytes;
    char* buffer;
    char* p;

    char str[100];
    size_t sent;

    method_str  = http_map_get(ct->dict, HTTP_METHOD);
    url_str     = http_map_get(ct->dict, HTTP_URL);
    version_str = http_map_get(ct->dict, HTTP_VERSION);
    if ((method_str == NULL) || (url_str == NULL) || (version_str == NULL)) {
        printf("ERROR: %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    if (_preprocess_request(conn, ct) < 0) {
        printf("ERROR: %s:%d\n", __FILE__, __LINE__);
        return -1;
    }

    sendfile_str = http_map_get(ct->dict, HTTP_SENDFILE);
    if (sendfile_str != NULL) {
        content_length = _get_file_size(sendfile_str);
        if (content_length <= 0) {
            printf("error get \"%s\" size!\n", sendfile_str);
        }
        sprintf(str, "%d", content_length);
        http_map_set(ct->dict, "Content-Length", str);

    } else if (ct->bodybuf->size > 0) {
        sprintf(str, "%d", ct->bodybuf->size);
        http_map_set(ct->dict, "Content-Length", str);
    }

    //reload string, because upper code modify maps!
    method_str  = http_map_get(ct->dict, HTTP_METHOD);
    url_str     = http_map_get(ct->dict, HTTP_URL);
    version_str = http_map_get(ct->dict, HTTP_VERSION);

    ////////////////////////////////////////////////////////
    //build request line
    buffer = (char*)http_malloc(HTTP_MAX_SEND_BLOCK);
    p = buffer;
    nr_bytes = sprintf(p, "%s %s %s\r\n",
            method_str, url_str, version_str);
    p += nr_bytes; 


    ////////////////////////////////////////////////////////
    //build headers
    for (http_map_begin(ct->dict, &iter);
        http_map_valid(&iter);
        http_map_next(&iter))
    {
        if ((iter.k == NULL) || (iter.v == NULL)) {
            continue;
        }

        //skip internal header
        //such like " VERSION", etc.
        if (iter.k[0] == ' ') {
            continue;
        }

        if (iter.v) {
            nr_bytes = sprintf(p, "%s: %s\r\n", iter.k, iter.v);
        } else {
            nr_bytes = sprintf(p, "%s:\r\n", iter.k);
        }
        p += nr_bytes;
    }

    *p++ = '\r';
    *p++ = '\n';

    ////////////////////////////////////////////////////////
    //send to remote
    socket_send(conn->handle, buffer, p - buffer, &sent, HTTP_SEND_TIMEOUT);

    if (sendfile_str != NULL) {
        sent += http_send_file(conn, sendfile_str);
    } else if (ct->body->size > 0) {
        sent += http_send_binary(conn, ct->body->ptr, ct->body->size);
    }

    http_free(buffer);
    buffer = NULL;

    return sent;
}



