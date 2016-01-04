# dukserver
A simple HTTP-Server base on Duktape


I want to build a remote mantain web server for ARM-based linux.
First I choose Python, but cross-compile is painful, and Python is very huge size...

So I decided to find some light script engine, such like lua.
But I don't like LUA Syntax...

Finally I found Duktape in my codes collection. And pick it up, try again.
So I base on luasocket and duktape, I implement a light http server.

But duktape is leak of buildin function. So I implement python like library.

Duktape Server:
* libhttpd/wsgi.txt: A example wsgi variable dump from mod_wsig+apache
* libhttpd/wsgi_server.c: Embedded duktape to libhttpd, base on http_mpm
* libhttpd/wsgi_server.h: Embedded duktape to libhttpd, base on http_mpm

Build Script:
* build/Makefile.msvc: Build script for Visual Studio 2010
* build/Makefile.mingw: Build script for MSYS mingw32
* build/Makefile.linux: Build script for Linux

Duktape files:
* duktape/duk_config.h: [https://github.com/svaarala/duktape]
* duktape/duktape.h: [https://github.com/svaarala/duktape]
* duktape/duktape.c: [https://github.com/svaarala/duktape]

Duktape python like library:
* duktape/subprocess.h: A python like popen.
* duktape/subprocess_win32.h: for Windows platform
* duktape/subprocess_linux.h: for Linux platform
* duktape/dukpylib.h: A python like library
* duktape/dukpylib.c: A python like library

Http/Httpd Library:
* libhttpd/http_buffer.h: A dynamic buffer interface
* libhttpd/http_client.c: Http Client interface
* libhttpd/http_client.h: Http Client interface
* libhttpd/http_common.h: Common define for this library
* libhttpd/http_connection.h: Connection data struct
* libhttpd/http_content.c: Common request, response data struct
* libhttpd/http_content.h: Common request, response data struct
* libhttpd/http_map.c: Simple map
* libhttpd/http_map.h: Simple map
* libhttpd/http_mpm.c: Multi-Thread MPM wrap server interface
* libhttpd/http_mpm.h: Multi-Thread MPM wrap server interface
* libhttpd/http_parse.c: Parser of headers
* libhttpd/http_parse.h: Parser of headers
* libhttpd/http_sendrecv.c: Request and Response interface
* libhttpd/http_sendrecv.h: Request and Response interface
* libhttpd/http_server.c: Http Server interface
* libhttpd/http_server.h: Http Server interface
* libhttpd/http_string.c: Some string function
* libhttpd/http_string.h: Some string function 
* libhttpd/http_sys.h: Abs layer of platform
* libhttpd/http_sys_linux.c: for Linux platform
* libhttpd/http_sys_w32.c: for Windows platform
* libhttpd/http_urldecode.c: decode url-encode-wwww
* libhttpd/http_urldecode.h: decode url-encode-wwww
* libhttpd/http_urlencode.c: TODO
* libhttpd/http_urlencode.h: TODO
* libhttpd/libhttp.c: user interface
* libhttpd/libhttp.h: user interface
