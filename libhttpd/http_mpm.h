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

#ifndef HTTP_MPM_H_4DC53B00_A3A9_11E5_BA55_005056C00008_INCLUDED_
#define HTTP_MPM_H_4DC53B00_A3A9_11E5_BA55_005056C00008_INCLUDED_

////////////////////////////////////////////////////////////
typedef struct http_mpm_handle_t http_mpm_handle_t;

typedef void            (*http_mpm_initialize_cb)(void* param);
typedef void            (*http_mpm_finalize_cb)(void* param);
typedef http_content_t* (*http_mpm_process_cb)(void* param, http_content_t* req);

////////////////////////////////////////////////////////////
http_mpm_handle_t* http_mpm_create(char* ip, int port);

void http_mpm_set_initialize(
        http_mpm_handle_t* h,
        http_mpm_initialize_cb cb,
        void* param);

void http_mpm_set_finalize(
        http_mpm_handle_t* h,
        http_mpm_finalize_cb cb);

void http_mpm_set_process(
        http_mpm_handle_t* h,
        http_mpm_process_cb cb);

http_bool_t http_mpm_start(http_mpm_handle_t* h);

void http_mpm_close(http_mpm_handle_t* h);

#endif

