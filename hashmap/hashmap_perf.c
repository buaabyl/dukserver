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

#include "hashmap.h"

void test_iter(void)
{
    hashmap_t* m;
    hashmap_iter_t iter;

    m = hashmap_create(100);
    hashmap_set(m, "1", "fish");
    hashmap_set(m, "2", "swordfish");
    hashmap_set(m, "3", "jewfish");
    hashmap_set(m, "4", "berryfish");

    for (hashmap_begin(m, &iter);hashmap_valid(&iter);hashmap_next(&iter)) {
        printf(" .key %s, .val %s\n", iter.key, iter.data);
    }

    hashmap_destroy(m);
}

void test(int max_slot, int max_elements)
{
    hashmap_t* m;
    int i;
    char key[16];
    int count = max_elements;
    int N = 1;
    clock_t clk1;
    clock_t clk2;
    clock_t clk_set = 0;
    clock_t clk_get = 0;
    clock_t clk_del = 0;

    while (N--) {
        m = hashmap_create(max_slot);

        clk1 = clock();
        for (i = 0;i < count;i++) {
            sprintf(key, "%08X", i);
            hashmap_set(m, key, key);
        }
        clk2 = clock();
        clk_set += clk2 - clk1;

        clk1 = clock();
        for (i = count-1;i >= 0;i--) {
            sprintf(key, "%08X", i);
            if (!hashmap_get(m, key)) {
                printf("Error not found when get \"%s\"\n", key);
            }
        }
        clk2 = clock();
        clk_get += clk2 - clk1;

        clk1 = clock();
        for (i = count-1;i >= 0;i--) {
            sprintf(key, "%08X", i);
            if (!hashmap_del(m, key)) {
                printf("Error not found when delete \"%s\"\n", key);
            }
        }
        clk2 = clock();
        clk_del += clk2 - clk1;

        //hashmap_dump(m);
        hashmap_destroy(m);
    }

    printf(" elements = %8d, slot = %8d, set = %10d, get = %10d, del = %10d\n",
            max_elements, max_slot,
            (int)clk_set, (int)clk_get, (int)clk_del);
}

int main(int argc, char* argv[])
{
    int max_elements;
    int max_slot;

    printf("test iter:\n");
    test_iter();

    printf("test performance:\n");
    for (max_elements = 1000;max_elements <= 100000;max_elements*=10) {
        for (max_slot = 0x10000;max_slot >= 1;max_slot >>= 1) {
            test(max_slot, max_elements);
        }
    }

    return 0;
}

