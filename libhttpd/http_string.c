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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"

char* http_strdup(const char* s)
{
    char* p;
    int n;

    n = strlen(s);

    p = (char*)http_malloc(n+1);
    memset(p, 0, n+1);
    if (n > 0) {
        memcpy(p, s, n);
    }

    return p;
}

char* http_strndup(const char* s, int n)
{
    char* p;

    p = (char*)http_malloc(n+1);
    memset(p, 0, n+1);
    if (n > 0) {
        memcpy(p, s, n);
    }

    return p;
}

const char* http_find_char(const char* s, char ch, int len)
{
    const char* pend;

    pend = s + len;

    while (s < pend) {
        if (*s == ch) {
            return s;
        }
        s++;
    }

    return NULL;
}

const char* http_rfind_char(const char* s, char ch, int len)
{
    const char* p;

    p = s + len-1;

    while (s <= p) {
        if (*p == ch) {
            return p;
        }
        p--;
    }

    return NULL;
}

const char* http_find_alpha(const char* s, int len)
{
    const char* p;
    const char* pend;

    p    = s;
    pend = s + len;

    while (p < pend) {
        if (isalpha(*p)) {
            break;
        }
        p++;
    }
    if (p == pend) {
        return NULL;
    }

    return p;
}

const char* http_find_nwhite(const char* s, int len)
{
    const char* p;
    const char* pend;

    p    = s;
    pend = s + len;

    while (p < pend) {
        if (!isspace(*p)) {
            return p;
        }
        p++;
    }

    return NULL;
}

const char* http_rfind_nwhite(const char* s, int len)
{
    const char* p;

    p    = s + len -1;

    while (s <= p) {
        if (!isspace(*p)) {
            return p;
        }
        p--;
    }

    return NULL;
}

const char* http_find_crlf(const char* pbuf, int len)
{
    while (len--) {
        if ((pbuf[0] == '\r') && 
            (pbuf[1] == '\n') &&
            (pbuf[2] == '\r') &&
            (pbuf[3] == '\n'))
        {
            return (char*)pbuf;
        }

        pbuf++;
    }

    return NULL;
}

const char* http_find_digit(const char* s, int len)
{
    const char* pend;

    pend = s + len;

    while (s < pend) {
        if (isdigit(*s)) {
            return s;
        }
        s++;
    }

    return NULL;
}

