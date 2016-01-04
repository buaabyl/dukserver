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

/*
 * first implement is hashlib, but it is useless.
 * so I decided to use array.
 *
 * this implement sometime act as std::vector,
 * user alloc initial size, when overflow, resize 2 bigger.
 *
 */

#ifndef HTTP_MAP_H_08F5BCCF_8E6D_11E5_88A5_005056C00008_INCLUDED_
#define HTTP_MAP_H_08F5BCCF_8E6D_11E5_88A5_005056C00008_INCLUDED_

////////////////////////////////////////////////////////////
typedef struct _http_pair_t{
    char* a;
    char* b;
}http_pair_t;

typedef struct _http_map_t{
    int             max_count;
    int             count;
    http_pair_t*    values;
}http_map_t;

http_map_t*  http_map_new(int maxcount);
void         http_map_delete(http_map_t* m);
http_pair_t* http_map_set(http_map_t* m, const char* k, const char* v);
const char*  http_map_get(http_map_t* m, const char* k);
http_bool_t  http_map_erase(http_map_t* m, const char* k);
int          http_map_count(http_map_t* m);

////////////////////////////////////////////////////////////
typedef struct _http_map_iter_t{
    http_map_t* parent;

    int index;
    const char* k;
    const char* v;
}http_map_iter_t;

http_bool_t http_map_begin(http_map_t* m, http_map_iter_t* iter);
http_bool_t http_map_valid(http_map_iter_t* iter);
http_bool_t http_map_next(http_map_iter_t* iter);

#endif

