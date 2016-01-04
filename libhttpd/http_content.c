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
#include "http_buffer.h"
#include "http_content.h"

http_content_t* http_content_new(void)
{
    http_content_t* ct;

    ct = (http_content_t*)http_malloc(sizeof(http_content_t));
    memset(ct, 0, sizeof(http_content_t));

    ct->headerbuf  = (http_buffer_t*)http_malloc(sizeof(http_buffer_t));
    ct->headerbuf->buf     = (char*)http_malloc(HTTP_MAX_BUFFER_SIZE);
    ct->headerbuf->maxsize = HTTP_MAX_BUFFER_SIZE;
    ct->headerbuf->size    = 0;

    ct->header  = (http_reference_t*)http_malloc(sizeof(http_reference_t));
    memset(ct->header, 0, sizeof(http_reference_t));
    ct->body    = (http_reference_t*)http_malloc(sizeof(http_reference_t));
    memset(ct->body, 0, sizeof(http_reference_t));

    ct->bodybuf = (http_buffer_t*)http_malloc(sizeof(http_buffer_t));
    ct->bodybuf->buf     = (char*)http_malloc(HTTP_MAX_BUFFER_SIZE);
    ct->bodybuf->maxsize = HTTP_MAX_BUFFER_SIZE;
    ct->bodybuf->size    = 0;

    ct->dict    = http_map_new(HTTP_DEFAULT_HEADERS_COUNT);

    return ct;
}

void http_content_delete(http_content_t* ct)
{
    const char* tmpfilename_str;
    FILE* fp;

    if (ct == NULL) {
        printf("http_content_delete error\n");
    }

    tmpfilename_str = http_map_get(ct->dict, HTTP_TMPFILE);
    if (tmpfilename_str != NULL) {
        if ((fp = fopen(tmpfilename_str, "r")) != NULL) {
            printf("remove \"%s\"\n", tmpfilename_str);
            fclose(fp);
            remove(tmpfilename_str);
        }
    }

    http_free(ct->headerbuf->buf);
    ct->headerbuf->buf = NULL;
    http_free(ct->headerbuf);
    ct->headerbuf = NULL;

    http_free(ct->header);
    ct->header = NULL;

    http_free(ct->body);
    ct->body = NULL;

    http_free(ct->bodybuf->buf);
    ct->bodybuf->buf = NULL;
    http_free(ct->bodybuf);
    ct->bodybuf = NULL;

    http_map_delete(ct->dict);
    ct->dict = NULL;

    http_free(ct);
}

void http_dump_content(http_content_t* ct)
{
    int i;
    char c;
    const char* tmpfilename_str;
    http_map_iter_t iter;

    printf("[DUMP]\n");
    for (http_map_begin(ct->dict, &iter);
        http_map_valid(&iter);
        http_map_next(&iter))
    {
        printf("%s:%s\n", iter.k, iter.v);
    }
    printf("\n");

    if (ct->body->ptr) {
        for (i = 0;i < ct->body->size;i++) {
            c = ct->body->ptr[i];
            if (c == '\r') {
                printf("\\r");
            } else if (c == '\n') {
                printf("\\n\n");
            } else {
                printf("%c", c);
            }
        }

    } else {
        tmpfilename_str = http_map_get(ct->dict, HTTP_TMPFILE);
        if (tmpfilename_str != NULL) {
            printf("[FILE]\"%s\"\n", tmpfilename_str);
        }
    }
    printf("[EOF]\n");
}

