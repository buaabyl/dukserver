# dukserver
A simple HTTP-Server base on Duktape


I want to build a remote mantain web server for ARM-based linux.
First I choose Python, but cross-compile is painful, and Python is very huge size.

So I decided to find some light script engine, such like lua.
But I don't like LUA Syntax.

Finally I found Duktape in my codes collection. And pick it up, try again.
So I base on luasocket and duktape, I implement a light http server.

But duktape is leak of buildin function.
So I implement python like library.
And extend String.encode(encoding) and String.decode(encodeing) base on libiconv.
And override default Duktape.modSearch function.


```javascript
  var fs = require('fs');
  var os = require('os');
  var path = require('os.path');
  var sys = require('sys');
  var time = requre('time');
  var _subprocess = require('_subprocess');
  
  //file access
  var f = fs.open('../../file.bin');
  var text = f.read();
  f.close();
  
  //reflaction method
  var m = dir(f);
  for (var k in m) {
      print(' ' + k + ': ' + m[k]);
  }
  
  print(" os.getcwd(): " + 
        os.getcwd());
    
  print(" os.path.split('C:'): " + 
        os.path.split('C:'));
  print(" os.path.split('C:/'): " + 
        os.path.split('C:/'));
  print(" os.path.split('/ewfwef/ffff.js'): " + 
        os.path.split('/ewfwef/ffff.js'));

  print(" os.path.splitext('/ewfwef/ffffjs'): " + 
        os.path.splitext('/ewfwef/ffffjs'));
  print(" os.path.splitext('c:\\ewfwef\\ffff.js'): " + 
        os.path.splitext('c:\\ewfwef\\ffff.js'));
  print(" os.path.splitext('c:/ewfwef/ffff.js'): " + 
        os.path.splitext('c:/ewfwef/ffff.js'));

  print(" os.path.normpath('test/../.././hello.js'): " + 
        os.path.normpath('test/../.././hello.js'));

  print(" os.path.abspath('test/../.././hello.js'): " + 
        os.path.abspath('test/../.././hello.js'));

  print(" os.path.join('C:/', 'test', 'file.js')" +
        os.path.join('C:/', 'test', 'file.js'));
        
  //string encode, decode
  var str_utf8 = 'hello world';
  var str_ucs2 = str_utf8.decode('UTF-8');
  var str_gbk  = str_utf8.encode('GBK');
  
  //subprocess
  var ps = _subprocess.psopen('ls', '-al');
  while (1) {
      var out = ps.stdout.read();
      if (out == null) {
        break;
      }
      print(out);
  }
  ps.wait();
  ps.close();
```

Duktape Server:
* libhttpd/wsgi.txt: A example wsgi variable dump from mod_wsig+apache
* libhttpd/wsgi_server.c: Embedded duktape to libhttpd, base on http_mpm
* libhttpd/wsgi_server.h: Embedded duktape to libhttpd, base on http_mpm

Build Script:
* build/Makefile.msvc: Build script for Visual Studio 2010
* build/Makefile.mingw: Build script for MSYS mingw32
* build/Makefile.linux: Build script for Linux

Duktape files [https://github.com/svaarala/duktape]:
* duktape/duk_config.h
* duktape/duktape.h
* duktape/duktape.c

Duktape python like library:
* duktape/subprocess.h: A python like popen.
* duktape/subprocess_win32.h: for Windows platform
* duktape/subprocess_linux.h: for Linux platform
* duktape/dukpylib.h: A python like library
* duktape/dukpylib.c: A python like library

LuaSocket [http://luaforge.net/projects/luasocket/]:
* libsocket/libsocket.h
* libsocket/usocket.c
* libsocket/wsocket.c

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
* libhttpd/http_urlencode.c: encode url-encode-www
* libhttpd/http_urlencode.h: encode url-encode-www
* libhttpd/libhttp.c: user interface
* libhttpd/libhttp.h: user interface
