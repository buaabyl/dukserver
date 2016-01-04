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
#include <time.h>

#include "duktape.h"
#include "dukpylib.h"

static int myduk_compile_execute(duk_context *ctx) 
{
	int comp_flags = 0;

	duk_compile(ctx, comp_flags);

	duk_push_global_object(ctx);  /* 'this' binding */
	duk_call_method(ctx, 0);

    if (!duk_is_null_or_undefined(ctx, -1)) {
        fprintf(stdout, "%s\n", duk_to_string(ctx, -1));
        fflush(stdout);
    }
	duk_pop(ctx);

	return 0;
}

static int myduk_get_stack_raw(duk_context *ctx) 
{
    const char* s;
    const char fmt[] = "ExceptionError: \"%s\"\n";
    char* p;

    ////////////////////////////////////////////////////////
    //'throw' Exception
    if (duk_is_string(ctx, -1)) {
        s = duk_to_string(ctx, -1);
        p = (char*)malloc(sizeof(fmt) + strlen(s) + 1);
        sprintf(p, fmt, s);
        duk_pop(ctx);
        duk_push_lstring(ctx, p, strlen(p));
        free(p);
        return 1;
    }

    ////////////////////////////////////////////////////////
    //Other exception
	if (!duk_is_object(ctx, -1)) {
		return 1;
	}
	if (!duk_has_prop_string(ctx, -1, "stack")) {
		return 1;
	}

	duk_get_prop_string(ctx, -1, "stack");  /* caller coerces */
	duk_remove(ctx, -2);

	return 1;
}

void myduk_print_prefix(int nindent)
{
    printf(".... ");

    while (nindent-- > 0) {
        printf("    ");//4 space
    }
}

void myduk_peval_stdin(duk_context* ctx)
{
    int rc;
    char c;
    int nlen;
    int nindent = 0;
    static char buffer[2048];

    while (1) {
        fprintf(stdout, "duk> ");
        fflush(stdout);
        nindent = 0;

        nlen = 0;
        while (1) {
            c = fgetc(stdin);
            if (nlen >= sizeof(buffer)-1) {
                nlen = -1;
                break;
            }
            if (c == EOF) {
                buffer[nlen] = '\0';
                break;
            }
            if (c == '\n') {
                if (nlen > 0) {
                    if (buffer[nlen-1] == '\n') {
                        buffer[nlen] = '\0';
                        break;
                    } else if ((nindent == 0) && (buffer[nlen-1] == ';')) {
                        buffer[nlen] = '\0';
                        break;
                    } else if (buffer[nlen-1] == '{') {
                        nindent++;
                        myduk_print_prefix(nindent);
                    } else if (buffer[nlen-1] == '}') {
                        nindent--;
                        myduk_print_prefix(nindent);
                    } else {
                        myduk_print_prefix(nindent);
                    }

                } else {//nlen <= 0
                    continue;
                }
            }
            buffer[nlen++] = c;
        }

        if (nlen == -1) {
            break;
        }
        if (nlen == 0) {
            if (c == EOF) {
                break;
            }
            continue;
        }

        duk_push_lstring(ctx, buffer, nlen);
        duk_push_string(ctx, "input");

        rc = duk_safe_call(ctx, myduk_compile_execute, 2 /*nargs*/, 1 /*nret*/);
        if (rc != DUK_EXEC_SUCCESS) {
            duk_safe_call(ctx, myduk_get_stack_raw, 1 /*nargs*/, 1 /*nrets*/);
            fprintf(stderr, "%s\n", duk_safe_to_string(ctx, -1));
            fflush(stderr);
        }
        duk_pop(ctx);

        if (c == EOF) {
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    duk_context* ctx;

    ctx = duk_create_heap_default();
    dukopen_pylib(ctx);

    pylib_put_args(ctx, argc, argv);

    if (argc == 1) {
        myduk_peval_stdin(ctx);
    } else {
        pylib_include(ctx, argv[1]);
    }

    duk_destroy_heap(ctx);

    return 0;
}

