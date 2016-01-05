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

static char always_safe_buf[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789_.-";
static int always_safe_len = sizeof(always_safe_buf)/sizeof(always_safe_buf[0]) - 1;


static char _to_hex(char c);


//quote('abc def') -> 'abc%20def'
//
//  Each part of a URL, e.g. the path info, the query, etc., has a
//  different set of reserved characters that must be quoted.
//
//  RFC 2396 Uniform Resource Identifiers (URI): Generic Syntax lists
//  the following reserved characters.
//
//  reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
//                "$" | ","
//
//  Each of these characters is reserved in some component of a URL,
//  but not necessarily in all of them.
//
//  By default, the quote function is intended for quoting the path
//  section of a URL.  Thus, it will not encode '/'.  This character
//  is reserved, but in typical usage the quote function is being
//  called on a path where the existing slash characters are used as
//  reserved characters.
//
//static char safe_chars_buf[] = "%/:=&?~#+!$,;'@()*[]|";
//static int safe_chars_len = sizeof(safe_chars_buf)/sizeof(safe_chars_buf[0]) - 1;

char _to_hex(char c)
{
    if ((0 <= c) && (c <= 9)) {
        return c + '0';
    } else {
        return c - 10 + 'A';
    }
}

//@retval strlen(*pbufout) + 1;
int http_quote(const char* buffer, int len, char** pbufout)
{
    const char* p;
    const char* pend;
    char* bufout;
    char* pwrite;
    int i;
    char c;

    bufout  = (char*)http_malloc(sizeof(char) * 3 * len + 1);
    pend    = buffer + len;
    p       = buffer;
    pwrite  = bufout;

    while (p < pend) {
        c = *p++;
        for (i = 0;i < always_safe_len;i++) {
            if (c == always_safe_buf[i]) {
                break;
            }
        }

        if (i != always_safe_len) {
            *pwrite++ = c;
        } else {
            *pwrite++ = '%';
            *pwrite++ = _to_hex(c >> 4);
            *pwrite++ = _to_hex(c & 0x0Fu);
        }
    }
    *pwrite++ = '\0';

    if (pbufout != NULL) {
        *pbufout = bufout;
    } else {
        http_free(bufout);
    }

    return pwrite - bufout;
}

int http_urlencode(http_map_t* m, char** pout)
{
    http_map_iter_t iter;
    int len;
    char* k;
    char* v;
    char* encoded_buf;
    int encoded_maxsize;
    int encoded_size;

    if (pout == NULL) {
        return -1;
    }

    encoded_maxsize = 1000;
    encoded_size = 0;
    encoded_buf = (char*)http_malloc(encoded_maxsize);

    for (http_map_begin(m, &iter);http_map_valid(&iter);http_map_next(&iter)) {
        //process key
        if ((iter.k == NULL) || (iter.k[0] == '\0')) {
            continue;
        }
        len = http_quote(iter.k, strlen(iter.k), &k);
        if (encoded_size + len >= encoded_maxsize) {
            encoded_maxsize *= 2;
            encoded_buf = (char*)http_realloc(encoded_buf, encoded_maxsize);
        }
        memcpy(encoded_buf + encoded_size, k, len);//len included '\0'
        encoded_size += len - 1;
        encoded_buf[encoded_size++] = '=';
        encoded_buf[encoded_size] = '\0';
        http_free(k);

        //process value
        if ((iter.v == NULL) || (iter.v[0] == '\0')) {
            continue;
        }
        len = http_quote(iter.v, strlen(iter.v), &v);
        if (encoded_size + len >= encoded_maxsize) {
            encoded_maxsize *= 2;
            encoded_buf = (char*)http_realloc(encoded_buf, encoded_maxsize);
        }
        memcpy(encoded_buf + encoded_size, v, len);//len included '\0'
        encoded_size += len - 1;
        encoded_buf[encoded_size++] = '&';
        encoded_buf[encoded_size] = '\0';
        http_free(k);
    }

    *pout = encoded_buf;

    return encoded_size;
}

