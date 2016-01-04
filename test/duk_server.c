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
#include "libhttp.h"
#include "duktape.h"
#include "dukpylib.h"
#include "wsgi_server.h"

void custom_init0(wsgi_handle_t* srv, void* param)
{
    printf("%s(%8p)\n", "custom_init0", param);
}

void custom_init1(wsgi_handle_t* srv, void* param)
{
    printf("%s(%8p)\n", "custom_init1", param);
}

void custom_fini0(wsgi_handle_t* srv, void* param)
{
    printf("%s(%8p)\n", "custom_fini0", param);
}

void custom_fini1(wsgi_handle_t* srv, void* param)
{
    printf("%s(%8p)\n", "custom_fini1", param);
}

int server(int argc, char* argv[], char* env[])
{
    wsgi_handle_t* h;

    if (argc <= 0) {
        printf("Need argument!\n");
        return -1;
    }

    h = wsgi_create("0.0.0.0", 1000);
    if (h == NULL) {
        return -1;
    }

    wsgi_add_initialize_chain(h, custom_init0, (void*)1);
    wsgi_add_finalize_chain(h, custom_fini0, (void*)2);

    wsgi_add_initialize_chain(h, custom_init1, (void*)3);
    wsgi_add_finalize_chain(h, custom_fini1, (void*)4);

    wsgi_start(h, argc, argv, env);

    getchar();

    wsgi_close(h);

    return 0;
}

int main(int argc, char* argv[], char* env[])
{
    socket_startup();
    http_init();

    server(argc-1, argv+1, env);

    if (http_memory_usage() > 0) {
        printf("*memory leak:\n");
        http_memory_dump();
    }

    http_fini();
    socket_cleanup();

    return 0;
}



