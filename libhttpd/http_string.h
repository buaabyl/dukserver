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

#ifndef HTUTILS_STRING_H_7A9A725E_5DC9_11E5_80A3_005056C00008_INCLUDED_
#define HTUTILS_STRING_H_7A9A725E_5DC9_11E5_80A3_005056C00008_INCLUDED_

char* http_strdup(const char* s);
char* http_strndup(const char* s, int n);

const char* http_find_char(const char* s, char ch, int len);
const char* http_rfind_char(const char* s, char ch, int len);

const char* http_find_alpha(const char* s, int len);

const char* http_find_nwhite(const char* s, int len);
const char* http_rfind_nwhite(const char* s, int len);

const char* http_find_crlf(const char* pbuf, int len);

const char* http_find_digit(const char* s, int len);


#endif

