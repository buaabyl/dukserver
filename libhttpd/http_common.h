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

#ifndef HTTP_COMMON_H_2CD7AE4F_8F31_11E5_8D78_005056C00008_INCLUDED_
#define HTTP_COMMON_H_2CD7AE4F_8F31_11E5_8D78_005056C00008_INCLUDED_


#define HTTP_DEFAULT_SERVER             "libhttpd/1.0.0"
#define HTTP_DEFAULT_CONTENT_TYPE       "text/plain; charset=UTF-8"
#define HTTP_DEFAULT_FORM_ENCODED       "application/x-www-form-urlencoded"

#define HTTP_DEFAULT_RESPONSE_EXPECT100 "HTTP/1.1 100 Continue\r\n\r\n"

//parsed result of "request line"
//also store in headers,
//but the key name is below:                         //request response
#define HTTP_METHOD                     " METHOD"    //  Y
#define HTTP_URL                        " URL"       //  Y
#define HTTP_SENDFILE                   " SENDFILE"  //  Y
#define HTTP_VERSION                    " VERSION"   //  Y       Y
#define HTTP_TMPFILE                    " TMPFILE"   //  Y       Y
#define HTTP_STATUS                     " STATUS"    //          Y
#define HTTP_MESSAGE                    " MESSAGE"   //          Y

//when reach this, restart fastcgi thread
#define HTTP_MPM_MAX_REQUESTS           1000

#define HTTP_DEFAULT_HEADERS_COUNT      1000

#define HTTP_ACCEPT_TIMEOUT             1   //second
#define HTTP_SEND_TIMEOUT               1   //second
#define HTTP_RECV_TIMEOUT               2   //second

#define HTTP_MPM_FASTCGI_TIMEOUT        100 //ms

#define K_BUFFER_1KB                    0x400
#define K_BUFFER_1MB                    0x100000

#define HTTP_MAX_BUFFER_SIZE            (10 * K_BUFFER_1KB)
#define HTTP_MAX_SEND_BLOCK             (2 * K_BUFFER_1MB)



#endif

