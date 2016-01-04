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

#ifndef HTUTILS_PARSE_H_80B56DC0_8C0F_11E5_B6F5_005056C00008_INCLUDED_
#define HTUTILS_PARSE_H_80B56DC0_8C0F_11E5_B6F5_005056C00008_INCLUDED_

http_bool_t http_parse_request_headers(
        const char* buffer,
        int buflen,
        http_map_t* hdr);

http_bool_t http_parse_response_headers(
        const char* buffer,
        int buflen,
        http_map_t* hdr);

#endif

