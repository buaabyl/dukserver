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

#ifndef LIBHTTP_H_46B44FD1_97F2_11E5_AEBE_005056C00008_INCLUDED_
#define LIBHTTP_H_46B44FD1_97F2_11E5_AEBE_005056C00008_INCLUDED_

#include "libsocket.h"

#include "http_common.h"
#include "http_sys.h"
#include "http_string.h"
#include "http_map.h"
#include "http_buffer.h"
#include "http_content.h"

#include "http_connection.h"
#include "http_client.h"
#include "http_server.h"
#include "http_sendrecv.h"
#include "http_parse.h"

#include "http_urlencode.h"
#include "http_urldecode.h"

#include "http_mpm.h"

extern const char HTTP_BUILD_VERSION[];

http_content_t* http_recv_request(http_connection_t* conn);
http_content_t* http_recv_response(http_connection_t* conn);

#endif

