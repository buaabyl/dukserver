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
 *
 *
 * hashmap is better for huge elements.
 * when elements < 1000, just use array or simple list.
 *
 * performance and space blance: max_slot = elements / 40
 *
 */

#ifndef HASHMAP_H_4AA8FF5E_AA16_11E5_A7A8_005056C00008_INCLUDED_
#define HASHMAP_H_4AA8FF5E_AA16_11E5_A7A8_005056C00008_INCLUDED_
typedef struct hashmap_list_t hashmap_list_t;
typedef struct hashmap_t hashmap_t;

typedef struct hashmap_iter_t{
    hashmap_t*          m;
    int                 idx_slot;
    hashmap_list_t*     idx_list;

    const char*         key;
    const char*         data;
}hashmap_iter_t;


hashmap_t*  hashmap_create(int max_slot);
void        hashmap_destroy(hashmap_t* m);

int         hashmap_set(hashmap_t* m, const char* k, const char* v);
const char* hashmap_get(hashmap_t* m, const char* k);
int         hashmap_del(hashmap_t* m, const char* k);

void        hashmap_dump(hashmap_t* m);

void        hashmap_begin(hashmap_t* m, hashmap_iter_t* iter);
int         hashmap_valid(hashmap_iter_t* iter);
void        hashmap_next(hashmap_iter_t* iter);


#endif

