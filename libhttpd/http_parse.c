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

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_map.h"

#include "http_parse.h"

typedef enum {
    _PARSE_REQUEST,
    _PARSE_RESPONSE
}_http_prase_t;

static http_bool_t _http_parse_request_line(
        const char* buf,
        int len,
        http_map_t* hdr);

static http_bool_t _http_parse_response_line(
        const char* buf,
        int len,
        http_map_t* hdr);

static http_bool_t _http_parse_header(
        const char* buf,
        int len,
        http_map_t* hdr);

static http_bool_t _http_parse_lines(
        _http_prase_t type,
        const char* buffer,
        int buflen,
        http_map_t* hdr);

http_bool_t _http_parse_request_line(const char* buf, int len, http_map_t* hdr)
{
    const char* s;
    const char* pend;
    const char* p;
    char* v;

    pend = buf + len;

    ////////////////////////////////////////////////////////
    s = http_find_alpha(buf, len);
    if (s == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    ////////////////////////////////////////////////////////
    //find 'method' end
    p = http_find_char(s, ' ', pend-s);
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }
    v = http_strndup(s, p-s);
    http_map_set(hdr, HTTP_METHOD, v);
    http_free(v);

    s = p+1;
    if (s >= pend) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    ////////////////////////////////////////////////////////
    //find '/'
    p = http_find_char(s, '/', pend-s);
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }
    s = p;

    //find 'url' end
    p = http_find_char(s, ' ', pend-s);
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }
    v = http_strndup(s, p-s);
    http_map_set(hdr, HTTP_URL, v);
    http_free(v);

    s = p+1;
    if (s >= pend) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    ////////////////////////////////////////////////////////
    //find 'HTTP/..'
    p = strstr(s, "HTTP/");
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    //only accept 1.0 or 1.1
    if (strncmp(p, "HTTP/1.1", 8) == 0) {
        http_map_set(hdr, HTTP_VERSION, "HTTP/1.1");

    } else if (strncmp(p, "HTTP/1.0", 8) == 0) {
        http_map_set(hdr, HTTP_VERSION, "HTTP/1.0");

    } else {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    return HTTP_TRUE;
}

http_bool_t _http_parse_response_line(const char* buf, int len, http_map_t* hdr)
{
    const char* s;
    const char* pend;
    const char* p;
    char* v;

    pend = buf + len;

    ////////////////////////////////////////////////////////
    s = http_find_alpha(buf, len);
    if (s == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    ////////////////////////////////////////////////////////
    //find 'HTTP/..'
    p = strstr(s, "HTTP/");
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    //only accept 1.0 or 1.1
    if (strncmp(p, "HTTP/1.1", 8) == 0) {
        http_map_set(hdr, HTTP_VERSION, "1.1");

    } else if (strncmp(p, "HTTP/1.0", 8) == 0) {
        http_map_set(hdr, HTTP_VERSION, "1.0");

    } else {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }
    s = p + 8;


    ////////////////////////////////////////////////////////
    //find 'digit'
    p = http_find_digit(s, pend-s);
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }
    s = p;

    p = http_find_char(s, ' ', pend-s);
    if (p == NULL) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    v = http_strndup(s, p-s);
    http_map_set(hdr, HTTP_STATUS, v);
    http_free(v);

    s = p+1;
    if (s >= pend) {
        printf("ERROR:%s:%d\n", __FILE__, __LINE__);
        return HTTP_FALSE;
    }

    ////////////////////////////////////////////////////////
    //message
    v = http_strndup(s, pend-s);
    http_map_set(hdr, HTTP_MESSAGE, v);
    http_free(v);

    return HTTP_TRUE;
}

http_bool_t _http_parse_header(const char* buf, int len, http_map_t* hdr)
{
    const char* s;
    const char* pend;
    const char* pcolon;
    const char* p;

    char* k = NULL;
    char* v = NULL;

    pend = buf + len;

    ////////////////////////////////////////////////////////
    s = http_find_alpha(buf, len);
    if (s == NULL) {
        return HTTP_FALSE;
    }

    pcolon  = http_find_char(s, ':', pend-s);
    if (pcolon == NULL) {
        return HTTP_FALSE;
    }
    p = pcolon;
    while (p[-1] == ' ') {
        p--;
    }
    k = http_strndup(s, p-s);

    ////////////////////////////////////////////////////////
    s = http_find_nwhite(pcolon+1, pend-pcolon-1);
    if (s != NULL) {
        p = http_rfind_nwhite(s, pend-s);
        v = http_strndup(s, p+1-s);
    }

    http_map_set(hdr, k, v);
    http_free(k);
    if (v) {
        http_free(v);
    }

    return HTTP_TRUE;
}

http_bool_t _http_parse_lines(
        _http_prase_t type,
        const char* buffer,
        int buflen,
        http_map_t* hdr)
{
    const char* p;
    const char* pend;
    const char* linebuf;
    int lineno;
    int len;
    int ret;

    lineno  = 0;
    pend    = buffer + buflen-1;
    linebuf = buffer;

    //walk through lines
    for (p = buffer;p < pend;p++) {
        if ((p[0] == '\r') && (p[1] == '\n')) {
            len = p - linebuf;
            if (len <= 0) {//CRLF
                break;
            }

            if (lineno == 0) {
                if (type == _PARSE_REQUEST) {
                    ret = _http_parse_request_line(linebuf, len, hdr);
                } else {
                    ret = _http_parse_response_line(linebuf, len, hdr);
                }
            } else {
                ret = _http_parse_header(linebuf, len, hdr);
            }

            if (!ret) {
                return HTTP_FALSE;
            }

            linebuf = p+2;
            lineno++;
        }
    }

    return HTTP_TRUE;
}

http_bool_t http_parse_request_headers(const char* buffer, int buflen, http_map_t* hdr)
{
    return _http_parse_lines(_PARSE_REQUEST, buffer, buflen, hdr);
}

http_bool_t http_parse_response_headers(const char* buffer, int buflen, http_map_t* hdr)
{
    return _http_parse_lines(_PARSE_RESPONSE, buffer, buflen, hdr);
}

