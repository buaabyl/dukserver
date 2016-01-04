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

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_map.h"

http_map_t* http_map_new(int maxcount)
{
    http_map_t* m;

    if (maxcount <= 0) {
        return NULL;
    }

    m = (http_map_t*)http_malloc(sizeof(http_map_t));
    m->max_count = maxcount;
    m->count     = 0;
    m->values    = (http_pair_t*)http_malloc(sizeof(http_pair_t)*maxcount);
    memset(m->values, 0, sizeof(http_pair_t)*maxcount);

    return m;
}

void http_map_delete(http_map_t* m)
{
    int i;
    http_pair_t* p;

    if (m == NULL) {
        return;
    }

    for (i = 0;i < m->count;i++) {
        p = &m->values[i];
        if (p->a) {
            http_free(p->a);
            p->a = NULL;
        }
        if (p->b) {
            http_free(p->b);
            p->b = NULL;
        }
    }

    http_free(m->values);
    m->values = NULL;

    http_free(m);
    m = NULL;
}

http_pair_t* http_map_set(http_map_t* m, const char* k, const char* v)
{
    int i;
    int max_count;
    http_pair_t* p;

    if ((m == NULL) || (k == NULL)) {
        return NULL;
    }

    //expand buffer
    if (m->count + 1 >= m->max_count) {
        max_count = m->max_count * 2;
        m->values = (http_pair_t*)http_realloc(
                m->values, sizeof(http_pair_t) * max_count);
        for (i = m->max_count;i < max_count;i++) {
            p = &m->values[i];
            p->a = NULL;
            p->b = NULL;
        }
        m->max_count = max_count;
    }

    //lookup duplicated key name
    for (i = 0;i < m->count;i++) {
        p = &m->values[i];
        if (strcmp(k, p->a) == 0) {
            if (p->b) {
                http_free(p->b);
                p->b = NULL;
            }
            if (v != NULL) {
                p->b = http_strdup(v);
            }
            return p;
        }
    }

    p = &m->values[m->count++];
    p->a = http_strdup(k);
    p->b = NULL;
    if (v != NULL) {
        p->b = http_strdup(v);
    }

    return p;
}

const char* http_map_get(http_map_t* m, const char* k)
{
    int i;
    http_pair_t* p;

    if ((m == NULL) || (k == NULL) || (m->count <= 0)) {
        return NULL;
    }

    for (i = 0;i < m->count;i++) {
        p = &m->values[i];
        if (strcmp(k, p->a) == 0) {
            return p->b;
        }
    }

    return NULL;
}

http_bool_t http_map_erase(http_map_t* m, const char* k)
{
    int i;
    http_pair_t* p;

    if ((m == NULL) || (k == NULL) || (m->count <= 0)) {
        return HTTP_FALSE;
    }

    //lookup
    for (i = 0;i < m->count;i++) {
        p = &m->values[i];
        if (strcmp(k, p->a) == 0) {
            break;
        }
    }
    if (i == m->count) {
        return HTTP_FALSE;
    }

    //found, so delete it
    if (p->b) {
        http_free(p->b);
        p->b = NULL;
    }
    http_free(p->a);
    p->a = NULL;

    //move
    for (;i < m->count-1;i++) {
        m->values[i].a = m->values[i+1].a;
        m->values[i].b = m->values[i+1].b;
    }
    m->values[m->count-1].a = NULL;
    m->values[m->count-1].b = NULL;
    m->count--;

    return HTTP_TRUE;
}

int http_map_count(http_map_t* m)
{
    return m->count;
}

http_bool_t http_map_begin(http_map_t* m, http_map_iter_t* iter)
{
    if (m->count <= 0) {
        iter->parent= NULL;
        iter->index = -1;
        iter->k     = NULL;
        iter->v     = NULL;
        return HTTP_FALSE;
    }

    iter->parent    = m;
    iter->index     = 0;
    iter->k         = m->values[0].a;
    iter->v         = m->values[0].b;

    return HTTP_TRUE;
}

http_bool_t http_map_valid(http_map_iter_t* iter)
{
    if (iter->index < 0) {
        return HTTP_FALSE;
    }

    return HTTP_TRUE;
}

http_bool_t http_map_next(http_map_iter_t* iter)
{
    if (iter->index == -1) {
        return HTTP_FALSE;
    }
    if (iter->index >= iter->parent->count - 1) {
        iter->index = -1;
        return HTTP_FALSE;
    }

    iter->index++;
    iter->k = iter->parent->values[iter->index].a;
    iter->v = iter->parent->values[iter->index].b;

    return HTTP_TRUE;
}







