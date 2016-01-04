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

#ifndef DUKPYLIB_H_FBD97D40_A148_11E5_97F3_001F160E9257_INCLUDED_
#define DUKPYLIB_H_FBD97D40_A148_11E5_97F3_001F160E9257_INCLUDED_

//linux maybe utf-8
#ifdef DUK_F_WINDOWS
#define DUKLIB_DEFAULT_SYSENCODING    "GBK"
#endif


char* pylib_iconv(duk_context* ctx, const char* instr, int inlen, int* outlen,
        const char* inenc, const char* outenc);

int pylib_include(duk_context* ctx, const char* fn);
int pylib_eval_string(duk_context* ctx, const char* s);

int pylib_put_args(duk_context* ctx, int argc, char* argv[]);

//nr_vars DUK_VARARGS
int pylib_put_c_method(duk_context* ctx,
        const char* function_name,
        duk_c_function function_entry);

int pylib_put_c_function(duk_context* ctx,
        const char* function_name,
        duk_c_function function_entry);

const char* pylib_type_name(duk_context* ctx, duk_idx_t index);

//encode or decode string on stack[idx]
int pylib_encode(duk_context* ctx, int idx, const char* encoding);
int pylib_decode(duk_context* ctx, int idx, const char* encoding);


int dukopen_pylib(duk_context* ctx);

#endif

