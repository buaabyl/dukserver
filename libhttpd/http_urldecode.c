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

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_map.h"

static int _parse_hex(const char* s, int len);


int _parse_hex(const char* s, int len)
{
    int v = 0;
    char c;

    while (len-- > 0) {
        c = tolower(*s++);
        if (('0' <= c) && (c <= '9')) {
            c = c - '0';
        } else if (('a' <= c) && (c <= 'f')) {
            c = c - 'a' + 10;
        } else {
            return 0;
        }

        v = v * 16 + c;
    }

    return v;
}

int http_unquote(char* buffer, int len)
{
    char* pend;
    char* pwrite;
    char* p;
    char c;

    p = buffer;
    pwrite = buffer;
    pend = buffer + len;

    while (p < pend) {
        if (*p == '+') {
            *pwrite++ = ' ';
            p++;

        } else if (*p == '%') {
            if (pend - p <= 2) {
                printf("ERROR\n");
                return -1;
            }
            c = _parse_hex(p+1, 2);
            *pwrite++ = c;
            p = p + 3;

        } else {
            *pwrite++ = *p++;
        }
    }

    if (pwrite != pend) {
        *pwrite++ = '\0';
    }

    return pwrite - buffer;
}

http_map_t* http_urldecode(char* buffer, int len)
{
    http_map_t* m;

    char* pend;
    char* s;
    char* p;

    char* key = NULL;
    char* value = NULL;

    pend = buffer + len;
    s = buffer;
    p = buffer;

    m = http_map_new(HTTP_DEFAULT_HEADERS_COUNT);

    while (p < pend) {
        if (*p == '=') {
            key = http_strndup(s, p-s);
            http_unquote(key, p-s);
            s = ++p;

        } else if (*p == '&') {
            if (s < p) {
                if (key) {
                    value = http_strndup(s, p-s);
                    http_unquote(value, p-s);
                    http_map_set(m, key, value);
                } else {
                    key   = http_strndup(s, p-s);
                    http_unquote(key, p-s);
                    http_map_set(m, key, "");
                }

            } else if (key != NULL) {
                http_map_set(m, key, "");
            }

            if (key) {
                http_free(key);
                key = NULL;
            }
            if (value) {
                http_free(value);
                value = NULL;
            }

            s = ++p;

        } else {
            p++;
        }
    }

    if (s < p) {
        if (key) {
            value = http_strndup(s, p-s);
            http_unquote(value, p-s);
            http_map_set(m, key, value);
        } else {
            key   = http_strndup(s, p-s);
            http_unquote(key, p-s);
            http_map_set(m, key, "");
        }

    } else if (key != NULL) {
        http_map_set(m, key, "");
    }

    //cleanup
    if (key) {
        http_free(key);
        key = NULL;
    }
    if (value) {
        http_free(value);
        value = NULL;
    }

    return m;
}


