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
#include <errno.h>
#include <iconv.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "duktape.h"
#include "dukpylib.h"

#ifdef DUK_F_WINDOWS 
#include <windows.h>
#include <direct.h>
#include <io.h>
#endif

#ifdef DUK_F_LINUX
#include <unistd.h>
#endif

typedef struct stat stat_t;

#include "subprocess.h"

//Changelog
//=========
//
//2015.12.13    copy from Library/duktape
//2015.12.13    fix misuse of 'duk_to_string' and 'duk_to_lstring'
//              fix misuse of 'duk_to_string' to 'duk_get_string'
//              fix duk_write because misuse of duk_get_length.
//2015.12.22    new map() object, modify dir() function
//2015.12.23    add foreach to map
//2015.12.29    delete map implement.
//
//2015.12.30    python like stdlib:
//                  'os', 'os.path', 'sys', 'time'
//                  'fs'
//2015.12.31    add os.system, abspath lookup work directory first 
//              when pattern is begin with "./"
//
//2016.01.02    finish _subprocess under windows and linux.
//2016.01.03    move all extend module to global['Modules']
//              and set default modSearch function.
//

////////////////////////////////////////////////////////////
//call :func(p0, p1);
//           ^   ^
//index:     0   1
//index:    -2  -1
//
//map: set, get, del, foreach
//Array: push, pop, splice(delete)
//
//
static int _helper_get_stack_raw(duk_context* ctx);
static void _helper_add_dollar_prefix(duk_context* ctx, int idx);
static void _helper_unpack_statobj(duk_context* ctx, stat_t* st, int idx);
static void _helper_pack_statobj(duk_context* ctx, stat_t* st, int idx);
static void _helper_unpack_timeobj(duk_context* ctx, struct tm* ts, int idx);
static void _helper_pack_timeobj(duk_context* ctx, struct tm* ts, int idx);
static int _helper_path_normpath(duk_context* ctx, char* s, int len);

static int _strobj_encode(duk_context* ctx);
static int _strobj_decode(duk_context* ctx);
static int _fileobj_close(duk_context* ctx);
static int _fileobj_read(duk_context* ctx);
static int _fileobj_write(duk_context* ctx);
static int _statobj_tostring(duk_context* ctx);
static int _timeobj_tostring(duk_context* ctx);

static int _ord(duk_context* ctx);
static int _chr(duk_context* ctx);
static int _hex(duk_context* ctx);
static int _dir(duk_context* ctx);
static int _globals(duk_context* ctx);
static int _include(duk_context* ctx);
static int _deepcopy1(duk_context* ctx);

static int _fs_open(duk_context* ctx);
static int _fs_file_put_content(duk_context* ctx);
static int _fs_file_get_content(duk_context* ctx);

static int _os_stat(duk_context* ctx);
static int _os_getcwd(duk_context* ctx);
static int _ospath_isdir(duk_context* ctx);
static int _ospath_isfile(duk_context* ctx);
static int _ospath_exists(duk_context* ctx);
static int _ospath_join(duk_context* ctx);
static int _ospath_dirname(duk_context* ctx);
static int _ospath_split(duk_context* ctx);
static int _ospath_splitext(duk_context* ctx);
static int _ospath_normpath(duk_context* ctx);
static int _ospath_abspath(duk_context* ctx);

static int _time_time(duk_context* ctx);
static int _time_localtime(duk_context* ctx);
static int _time_gmtime(duk_context* ctx);
static int _time_asctime(duk_context* ctx);
static int _time_ctime(duk_context* ctx);
static int _time_strftime(duk_context* ctx);

static void _dukopen_buildin_extend(duk_context* ctx);
static void _push_traces_obj(duk_context* ctx);
static void _push_fs_obj(duk_context* ctx);
static void _push_os_path_obj(duk_context* ctx);
static void _push_os_obj(duk_context* ctx);
static void _push_time_obj(duk_context* ctx);
static void _push_sys_obj(duk_context* ctx);
static void _push_subprocess_obj(duk_context* ctx);
static void _set_modsearch(duk_context* ctx);

//#define duk_alloc_raw(c, size)        malloc(size)
//#define duk_realloc_raw(c, p, size)   realloc(p, size)
//#define duk_free_raw(c, p)            free(p)

////////////////////////////////////////////////////////////
int _helper_get_stack_raw(duk_context* ctx) 
{
    const char* s;
    const char fmt[] = "ExceptionError: \"%s\"\n";
    char* p;

    ////////////////////////////////////////////////////////
    //'throw "string"' Exception
    if (duk_is_string(ctx, -1)) {
        s = duk_get_string(ctx, -1);
        p = (char*)duk_alloc_raw(ctx, sizeof(fmt) + strlen(s) + 1);
        sprintf(p, fmt, s);
        duk_pop(ctx);
        duk_push_lstring(ctx, p, strlen(p));
        duk_free_raw(ctx, p);
        return 1;
    }

    ////////////////////////////////////////////////////////
    //Other exception is object, and object have property named "stack"
	if (!duk_is_object(ctx, -1)) {
		return 1;
	}
	if (!duk_has_prop_string(ctx, -1, "stack")) {
		return 1;
	}

    //...|object|

	duk_get_prop_string(ctx, -1, "stack");  /* caller coerces */
    //...|object|message|

	duk_remove(ctx, -2);//delete object
    //...|message|

	return 1;
}

void _helper_add_dollar_prefix(duk_context* ctx, int idx)
{
    const char* k;
    char* newk;
    duk_size_t n;

    if (idx < 0) {
        idx = duk_get_top(ctx) + idx;
    }

    k = duk_get_lstring(ctx, idx, &n);
    newk = (char*)duk_alloc_raw(ctx, n + 1);
    memset(newk, 0, n + 1);
    newk[0] = '$';
    memcpy(&newk[1], k, n);
    duk_push_lstring(ctx, newk, n + 1);
    duk_free_raw(ctx, newk);

    duk_replace(ctx, idx);
}

void _helper_unpack_statobj(duk_context* ctx, stat_t* st, int idx)
{
    duk_get_prop_string(ctx, idx, "st_dev");
    st->st_dev = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_ino");
    st->st_ino = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_mode");
    st->st_mode = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_nlink");
    st->st_nlink = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_uid");
    st->st_uid = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_gid");
    st->st_gid = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_size");
    st->st_size = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_atime");
    st->st_atime = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_mtime");
    st->st_mtime = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "st_ctime");
    st->st_ctime = duk_to_int(ctx, -1);
    duk_pop(ctx);
}

void _helper_pack_statobj(duk_context* ctx, stat_t* st, int idx)
{
    duk_push_int(ctx, st->st_dev);
    duk_put_prop_string(ctx, idx-1, "st_dev");

    duk_push_int(ctx, st->st_ino);
    duk_put_prop_string(ctx, idx-1, "st_ino");

    duk_push_int(ctx, st->st_mode);
    duk_put_prop_string(ctx, idx-1, "st_mode");

    duk_push_int(ctx, st->st_nlink);
    duk_put_prop_string(ctx, idx-1, "st_nlink");

    duk_push_int(ctx, st->st_uid);
    duk_put_prop_string(ctx, idx-1, "st_uid");

    duk_push_int(ctx, st->st_gid);
    duk_put_prop_string(ctx, idx-1, "st_gid");

    duk_push_int(ctx, st->st_size);
    duk_put_prop_string(ctx, idx-1, "st_size");

    duk_push_int(ctx, (int)st->st_atime);
    duk_put_prop_string(ctx, idx-1, "st_atime");

    duk_push_int(ctx, (int)st->st_mtime);
    duk_put_prop_string(ctx, idx-1, "st_mtime");

    duk_push_int(ctx, (int)st->st_ctime);
    duk_put_prop_string(ctx, idx-1, "st_ctime");
}

void _helper_unpack_timeobj(duk_context* ctx, struct tm* ts, int idx)
{
    duk_get_prop_string(ctx, idx, "tm_year");
    ts->tm_year = duk_to_int(ctx, -1) - 1900;
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_mon");
    ts->tm_mon = duk_to_int(ctx, -1) - 1;
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_mday");
    ts->tm_mday = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_hour");
    ts->tm_hour = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_min");
    ts->tm_min = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_sec");
    ts->tm_sec = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_wday");
    ts->tm_wday = duk_to_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_yday");
    ts->tm_yday = duk_to_int(ctx, -1) - 1;
    duk_pop(ctx);

    duk_get_prop_string(ctx, idx, "tm_isdst");
    ts->tm_isdst = duk_to_int(ctx, -1);
    duk_pop(ctx);
}

void _helper_pack_timeobj(duk_context* ctx, struct tm* ts, int idx)
{
    duk_push_int(ctx, ts->tm_year + 1900);
    duk_put_prop_string(ctx, idx-1, "tm_year");

    duk_push_int(ctx, ts->tm_mon + 1);
    duk_put_prop_string(ctx, idx-1, "tm_mon");

    duk_push_int(ctx, ts->tm_mday);
    duk_put_prop_string(ctx, idx-1, "tm_mday");

    duk_push_int(ctx, ts->tm_hour);
    duk_put_prop_string(ctx, idx-1, "tm_hour");

    duk_push_int(ctx, ts->tm_min);
    duk_put_prop_string(ctx, idx-1, "tm_min");
    
    duk_push_int(ctx, ts->tm_sec);
    duk_put_prop_string(ctx, idx-1, "tm_sec");

    duk_push_int(ctx, ts->tm_wday);
    duk_put_prop_string(ctx, idx-1, "tm_wday");

    duk_push_int(ctx, ts->tm_yday + 1);
    duk_put_prop_string(ctx, idx-1, "tm_yday");

    duk_push_int(ctx, ts->tm_isdst);
    duk_put_prop_string(ctx, idx-1, "tm_isdst");
}

//@param s      no mater with or without '\0'
//@param len    length not included '\0'
int _helper_path_normpath(duk_context* ctx, char* s, int len)
{
    char** stack;
    int max_depth = 128;
    int depth = 0;
    char* p;
    char* prev;
    char* pend;
    int n;
    int i;
    int j;
#ifdef DUK_F_WINDOWS
    const char split_char = '\\';
#endif
#ifdef DUK_F_LINUX
    const char split_char = '/';
#endif

    stack = (char**)duk_alloc_raw(ctx, sizeof(char*) * max_depth);

    prev    = s;
    pend    = s + len;
    p       = prev;
    while (p < pend) {
        if (depth == max_depth - 1) {
            max_depth *= 2;
            stack = (char**)duk_realloc_raw(ctx, stack, sizeof(char*) * max_depth);
        }

        if ((*p == '/') || (*p == '\\')) {
            if (prev < p) {
                n = p-prev;
                stack[depth] = (char*)duk_alloc_raw(ctx, n+1);
                memset(stack[depth], 0, n+1);
                memcpy(stack[depth], prev, n);
                depth++;
                prev = p+1;
                p = prev;
            } else {
                p++;
            }
        } else {
            p++;
        }
    }
    if (prev < p) {
        n = p-prev;
        stack[depth] = (char*)duk_alloc_raw(ctx, n+1);
        memset(stack[depth], 0, n+1);
        memcpy(stack[depth], prev, n);
        depth++;
    }

    i = 0;
    while (i < depth) {
        if (strcmp(stack[i], "..") == 0) {
            if (i > 0) {
                duk_free_raw(ctx, stack[i-1]);
                duk_free_raw(ctx, stack[i]);
                for (j = i+1;j < depth;j++) {
                    stack[j-2] = stack[j];
                }
                depth = depth-2;
                i--;
            } else {
                duk_free_raw(ctx, stack[i]);
                for (j = i+1;j < depth;j++) {
                    stack[j-1] = stack[j];
                }
                depth = depth-1;
            }

        } else if (strcmp(stack[i], ".") == 0) {
            duk_free_raw(ctx, stack[i]);
            for (j = i+1;j < depth;j++) {
                stack[j-1] = stack[j];
            }
            depth = depth-1;

        } else {
            i++;
        }
    }

    p = s;
    for (i = 0;i < depth;i++) {
        n = strlen(stack[i]);
        memcpy(p, stack[i], n);
        p += n;
        
        if (i < depth - 1) {
            *p++ = split_char;
        }

        duk_free_raw(ctx, stack[i]);
    }
    //when s not end with '\0',
    //still need to add '\0',
    //to prevent user just puts(s),
    //this will overflow!
    if (p < pend) {
        *p = '\0';
    }
    duk_free_raw(ctx, stack);

    return p - s;
}

int _strobj_encode(duk_context* ctx)
{
    const char* encoding;
    const char* s_ucs2;
    size_t n_ucs2;
    char* s_mbcs;
    size_t n_mbcs;

    char* inbuf;
    char* outbuf;
    size_t bytes_in;
    size_t bytes_left;

    iconv_t ctl;

    if ((duk_get_top(ctx) != 1) || !duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept encoding name");
    }

    encoding = duk_get_string(ctx, 0);
    if (strlen(encoding) == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not accept empty string");
    }

    ctl = iconv_open(encoding, "UCS-2LE");
    if (ctl <= 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }

    //get original string
    duk_push_this(ctx);
    s_ucs2 = duk_get_lstring(ctx, -1, &n_ucs2);//not-Null string!

    //prepare out buffer
    n_mbcs = n_ucs2 * 8;
    s_mbcs = duk_alloc_raw(ctx, n_mbcs);

    //convert
    inbuf  = (char*)s_ucs2;
    outbuf = s_mbcs;
    bytes_in   = n_ucs2;
    bytes_left = n_mbcs;

    if (iconv(ctl, &inbuf, &bytes_in, &outbuf, &bytes_left) != 0) {
        if (errno == EINVAL) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: An incomplete multibyte sequence is encountered in the input");
        }
        if (errno == E2BIG) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: The output buffer has no more room for the next converted character");
        }
        if (errno == EILSEQ) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: An invalid multibyte sequence is encountered in the input");
        }
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }
    n_mbcs = n_mbcs - bytes_left;

    duk_push_lstring(ctx, s_mbcs, n_mbcs);

    //cleanup
    duk_free_raw(ctx, s_mbcs);
    s_mbcs = NULL;

    iconv_close(ctl);
    ctl = 0;

    return 1;
}

int _strobj_decode(duk_context* ctx)
{
    const char* encoding;
    const char* s_mbcs;
    size_t n_mbcs;
    char* s_ucs2;
    size_t n_ucs2;

    char* inbuf;
    char* outbuf;
    size_t bytes_in;
    size_t bytes_left;

    iconv_t ctl;

    if ((duk_get_top(ctx) != 1) || !duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept encoding name");
    }

    encoding = duk_get_string(ctx, 0);
    if (strlen(encoding) == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not accept empty string");
    }

    ctl = iconv_open("UCS-2LE", encoding);
    if (ctl <= 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }

    //get original string
    duk_push_this(ctx);
    s_mbcs = duk_get_lstring(ctx, -1, &n_mbcs);//not-Null string!

    //prepare out buffer
    n_ucs2 = n_mbcs * 8;
    s_ucs2 = duk_alloc_raw(ctx, n_ucs2);

    //convert
    inbuf  = (char*)s_mbcs;
    outbuf = s_ucs2;
    bytes_in   = n_mbcs;
    bytes_left = n_ucs2;

    if (iconv(ctl, &inbuf, &bytes_in, &outbuf, &bytes_left) != 0) {
        if (errno == EINVAL) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: An incomplete multibyte sequence is encountered in the input");
        }
        if (errno == E2BIG) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: The output buffer has no more room for the next converted character");
        }
        if (errno == EILSEQ) {
            duk_error(ctx, DUK_ERR_INTERNAL_ERROR,
                    "iconv: An invalid multibyte sequence is encountered in the input");
        }
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }
    n_ucs2 = n_ucs2 - bytes_left;

    duk_push_lstring(ctx, s_ucs2, n_ucs2);

    //cleanup
    duk_free_raw(ctx, s_ucs2);
    s_ucs2 = NULL;

    iconv_close(ctl);
    ctl = 0;

    return 1;
}

// obj.close();
int _fileobj_close(duk_context* ctx)
{
    FILE* fp;

    //check parameters
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__handler__");
    fp = duk_get_pointer(ctx, -1);
    duk_pop(ctx);

    if (fp == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "fp of fclose(fp) is NULL!");
    }

    //close
    fclose(fp);

    return 0;
}

// obj.read(size);  --max read size
// obj.read();      --read all bytes
int _fileobj_read(duk_context* ctx)
{
    FILE* fp;
    unsigned char* rawbuf;
    unsigned char* dukbuf;
    unsigned long len;
    unsigned long pos;
    unsigned long fsize;
    unsigned long bytes_read;

    //check parameters
    if ((duk_get_top(ctx) == 1) && !duk_is_number(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "size must be interger");
        len = duk_to_uint32(ctx, 0);
    } else {
        len = 0;
    }

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__handler__");
    fp = duk_get_pointer(ctx, -1);
    duk_pop(ctx);

    if (fp == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "fp of fclose(fp) is NULL!");
    }

    //actual read
    pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    if ((len == 0) || (len > fsize - pos)) {
        len = fsize - pos;
    }

    //fixed bug:
    //  when fopen(fn, "r"), fread will convert "\r\n" to "\n",
    //  but fseek not check this!
    //  so we need to load this temporary then push to duktape.
    rawbuf = (unsigned char*)duk_alloc_raw(ctx, len);
    bytes_read = fread(rawbuf, sizeof(unsigned char), len, fp);

    dukbuf = (unsigned char*)duk_push_fixed_buffer(ctx, bytes_read);
    memcpy(dukbuf, rawbuf, bytes_read);
    duk_free_raw(ctx, rawbuf);

    return 1;
}


// obj.write(buffer);
// obj.write(string);
int _fileobj_write(duk_context* ctx)
{
    FILE* fp;
    unsigned char* buf;
    size_t len;
    unsigned long bytes_wrote;

    //check parameters
    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing buffer or string to write");
    }

    if (!duk_is_buffer(ctx, 0) && !duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "just buffer or string accept");
    }

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__handler__");
    fp = duk_get_pointer(ctx, -1);
    duk_pop(ctx);

    if (fp == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "fp of fclose(fp) is NULL!");
    }

    if (duk_is_buffer(ctx, 0)) {
        buf = (unsigned char*)duk_get_buffer(ctx, 0, &len);
    } else {//buf is NOT 'NUL-Terminated'!
        buf = (unsigned char*)duk_get_lstring(ctx, 0, &len);
    }

    if ((len <= 0) || (buf == NULL)) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "invalid buffer or string!");
    }

    bytes_wrote = fwrite(buf, sizeof(char), len, fp);
    duk_push_int(ctx, bytes_wrote);

    return 1;
}

int _statobj_tostring(duk_context* ctx)
{
    stat_t st;
    char buf[200];

    duk_push_this(ctx);
    _helper_unpack_statobj(ctx, &st, -1);
    duk_pop(ctx);

    sprintf(buf, "stat_result(st_mode=%d, st_ino=%d, st_dev=%d, st_nlink=%d, "
            "st_uid=%d, st_gid=%d, st_size=%d, "
            "st_atime=%d, st_mtime=%d, st_ctime=%d)",
            st.st_mode, (int)st.st_ino, (int)st.st_dev, (int)st.st_nlink,
            st.st_uid, st.st_gid, (int)st.st_size,
            (int)st.st_atime, (int)st.st_mtime, (int)st.st_ctime);

    duk_push_string(ctx, buf);

    return 1;
}

int _timeobj_tostring(duk_context* ctx)
{
    struct tm ts;
    char buf[100];

    duk_push_this(ctx);
    _helper_unpack_timeobj(ctx, &ts, -1);
    duk_pop(ctx);

    sprintf(buf,
            "time.struct_time(tm_year=%d, tm_mon=%d, tm_mday=%d, "
            "tm_hour=%d, tm_min=%d, tm_sec=%d, tm_wday=%d, "
            "tm_yday=%d, tm_isdst=%d)",
            ts.tm_year + 1900, ts.tm_mon + 1, ts.tm_mday,
            ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_wday,
            ts.tm_yday, ts.tm_isdst);

    duk_push_string(ctx, buf);

    return 1;
}

static int _psobj_stdin_write(duk_context* ctx)
{
    int f;
    const char* buf;
    duk_size_t size;
    int nr_bytes;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_RANGE_ERROR, "accept one argument only");
    }
    if (!duk_is_string(ctx, 0) && !duk_is_buffer(ctx, 0)) {
        duk_error(ctx, DUK_ERR_RANGE_ERROR, "need string or buffer");
    }

    if (duk_is_string(ctx, 0)) {
        buf = duk_get_lstring(ctx, 0, &size);
    } else {
        buf = duk_get_buffer(ctx, 0, &size);
    }

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__handler__");
    f = (int)duk_get_pointer(ctx, -1);
    duk_pop(ctx);

    nr_bytes = write(f, buf, size);
    if (nr_bytes != size) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "native write fail");
    }

    duk_push_int(ctx, nr_bytes);

    return 1;
}

static int _psobj_stdout_read(duk_context* ctx)
{
    char* buf = NULL;
    int size = 0;
    int nr_bytes;
    int f;

    if ((duk_get_top(ctx) != 0) && duk_is_number(ctx, 0)) {
        size = duk_get_int(ctx, 0);
    }

    if (size <= 0) {
        size = 1024;
    }

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__handler__");
    f = (int)duk_get_pointer(ctx, -1);
    duk_pop(ctx);

    buf = duk_alloc_raw(ctx, size);
    nr_bytes = read(f, buf, size);
    if (nr_bytes < 0) {
        duk_free(ctx, buf);
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "native read fail");
    }

    if (nr_bytes == 0) {
        duk_free(ctx, buf);
        duk_push_null(ctx);
        return 1;
    }

    duk_push_lstring(ctx, buf, nr_bytes);
    duk_free(ctx, buf);

    return 1;
}

static int _psobj_wait(duk_context* ctx)
{
    subprocess_t* ps;
    int returncode;

    duk_push_this(ctx);

    duk_get_prop_string(ctx, -1, "__handler__");
    if (duk_is_undefined(ctx, -1)) {
        ps = NULL;
    } else {
        ps = (subprocess_t*)duk_get_pointer(ctx, -1);
    }
    duk_pop(ctx);

    if (ps == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "missing '__handler__'!");
    }

    returncode = pswait(ps);

    duk_push_int(ctx, returncode);
    duk_put_prop_string(ctx, -2, "returncode");

    duk_push_int(ctx, returncode);
    return 1;
}

static int _psobj_close(duk_context* ctx)
{
    subprocess_t* ps;

    duk_push_this(ctx);

    duk_get_prop_string(ctx, -1, "__handler__");
    if (duk_is_undefined(ctx, -1)) {
        ps = NULL;
    } else {
        ps = (subprocess_t*)duk_get_pointer(ctx, -1);
    }
    duk_pop(ctx);

    if (ps == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "missing '__handler__'!");
    }

    psclose(ps);

    duk_push_undefined(ctx);
    duk_put_prop_string(ctx, -2, "__handler__");

    return 0;
}

static int _psobj_finalizer(duk_context* ctx)
{
    subprocess_t* ps;

    duk_get_prop_string(ctx, -1, "__handler__");
    if (!duk_is_undefined(ctx, -1)) {
        ps = (subprocess_t*)duk_get_pointer(ctx, -1);
        psclose(ps);
    }
    duk_pop(ctx);

    return 0;
}


int _ord(duk_context* ctx)
{
    const char* s;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_RANGE_ERROR, "just accept one argument!");
    }
    if (duk_get_type(ctx, -1) != DUK_TYPE_STRING) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "string only!");
    }

    s = duk_get_string(ctx, 0);
    if (strlen(s) == 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "null string not allow!");
    }

    duk_push_int(ctx, *s);

    return 1;
}

int _chr(duk_context* ctx)
{
    int v;
    char s[2];

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "one argument only!");
    }
    if (duk_get_type(ctx, -1) != DUK_TYPE_NUMBER) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "number only!");
    }

    v = duk_get_int(ctx, 0);
    s[0] = (char)v;
    s[1] = '\0';
    duk_push_string(ctx, s);

    return 1;
}

int _hex(duk_context* ctx)
{
    int v;
    char s[20];

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "one argument only!");
    }
    if (duk_get_type(ctx, -1) != DUK_TYPE_NUMBER) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "number only!");
    }

    v = duk_get_int(ctx, 0);
    sprintf(s, "0x%08x", v);
    duk_push_string(ctx, s);

    return 1;
}

//dir(obj)      DUK_ENUM_OWN_PROPERTIES_ONLY
//dir(obj, 1)   DUK_ENUM_INCLUDE_INTERNAL
//dir(obj, 2)   DUK_ENUM_INCLUDE_INTERNAL|DUK_ENUM_INCLUDE_NONENUMERABLE;
//
int _dir(duk_context* ctx)
{
    int depth;
    int obj_id = 0;
    int enum_level = 0;
    duk_uint_t enum_flags;

    depth = duk_get_top(ctx);
    if (depth == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "need one object!");
    }
    if (duk_get_type(ctx, 0) != DUK_TYPE_OBJECT) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "just accept object!");
    }

    if (depth == 2) {
        enum_level = duk_get_int(ctx, 1);
    }

    if (enum_level == 1) {
        enum_flags = DUK_ENUM_INCLUDE_INTERNAL;
    } else if (enum_level == 2) {
        enum_flags = DUK_ENUM_INCLUDE_INTERNAL|DUK_ENUM_INCLUDE_NONENUMERABLE;
    } else {
        enum_flags = DUK_ENUM_OWN_PROPERTIES_ONLY;
    }

    obj_id = duk_push_array(ctx);
    duk_enum(ctx, 0, enum_flags);

    // [o] [array] [enum]
    while (duk_next(ctx, -1/*enumid*/, 1/*get value*/)) {
        // [o] [array] [enum] key value
        _helper_add_dollar_prefix(ctx, -2);
        duk_put_prop(ctx, obj_id);
    }

    duk_pop(ctx);//pop [enum]

    return 1;
}

int _globals(duk_context* ctx)
{
    int depth = 0;

    depth = duk_get_top(ctx);
    if (depth > 0) {
        duk_pop_n(ctx, depth);
    }

    duk_push_global_object(ctx);
    if (depth > 0) {
        duk_push_int(ctx, 2);
    }
    _dir(ctx);

    return 1;
}

int _include(duk_context* ctx)
{
    const char* fn;
    duk_size_t n;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "need stript file name");
    }
    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }
    fn = duk_get_lstring(ctx, 0, &n);
    if (n <= 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "string is empty");
    }

    ////////////////////////////////////////////////////////
    //push '__file__' to 'traces'
    duk_push_global_object(ctx);
    duk_get_prop_string(ctx, -1, "__file__");

    duk_get_prop_string(ctx, -2, "Modules");
    duk_get_prop_string(ctx, -1, "traces");
    duk_remove(ctx, -2);

    if (duk_is_undefined(ctx, -1)) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "can't find 'traces'");
    }
    duk_push_string(ctx, "push");
    duk_dup(ctx, -3);
    duk_call_prop(ctx, -3, 1);
    duk_pop(ctx);

    //cleanup stack
    duk_pop(ctx);//pop 'traces'
    duk_remove(ctx, -2);//pop [global]

    ////////////////////////////////////////////////////////
    //stack: fn,__file__
    fn = duk_get_lstring(ctx, 0, &n);
    if ((n >= 2) && (fn[0] == '.') && ((fn[1] == '/') || (fn[1] == '\\'))) {
        duk_pop(ctx);

    } else if (duk_is_undefined(ctx, 1)) {
        duk_pop(ctx);

    } else {
        //tryfn = os.path.join(os.path.dirname(__file__), fn);
        //if os.path.exist(tryfn):
        //    fn = tryfn
        duk_push_c_function(ctx, _ospath_dirname, 1);
        duk_dup(ctx, -2);
        duk_call(ctx, 1);
        duk_remove(ctx, -2);

        duk_push_c_function(ctx, _ospath_join, 2);
        duk_dup(ctx, -2);
        duk_dup(ctx, -4);
        duk_call(ctx, 2);
        duk_remove(ctx, -2);

        duk_push_c_function(ctx, _ospath_exists, 1);
        duk_dup(ctx, -2);
        duk_call(ctx, 1);
        if (duk_get_boolean(ctx, -1)) {
            duk_pop(ctx);//pop result
            duk_remove(ctx, -2);//pop original string
        } else {
            duk_pop(ctx);//pop result
            duk_pop(ctx);//pop __file__
        }
    }

    ////////////////////////////////////////////////////////
    //stack: fn
    duk_push_c_function(ctx, _ospath_abspath, 1);
    duk_dup(ctx, -2);
    duk_call(ctx, 1);
    duk_remove(ctx, -2);

    //update '__file__'
    duk_push_global_object(ctx);
    duk_dup(ctx, -2);
    duk_put_prop_string(ctx, -2, "__file__");
    duk_pop(ctx);

#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, DUKLIB_DEFAULT_SYSENCODING);
#endif

    fn = duk_get_string(ctx, 0);
    if (duk_peval_file(ctx, fn) != 0) {
        duk_throw(ctx);
    }
    duk_remove(ctx, 0);//pop fn
    duk_gc(ctx, 0);

    ////////////////////////////////////////////////////////
    //restore '__file__'
    duk_push_global_object(ctx);

    duk_get_prop_string(ctx, -1, "Modules");
    duk_get_prop_string(ctx, -1, "traces");
    duk_remove(ctx, -2);

    duk_push_string(ctx, "pop");
    duk_call_prop(ctx, -2, 0);
    duk_remove(ctx, -2);//pop 'traces'

    duk_put_prop_string(ctx, -2, "__file__");

    duk_pop(ctx);//pop [global]

    return 1;
}

//copy method of $0 to $1
//
//equal:
//  var m = dir($0);
//  for (var k in m) {
//      $1[k.substring(1)] = m[k];
//  }
//
static int _deepcopy1(duk_context* ctx)
{
    int from_id = 0;
    int to_id = 1;

    if (duk_get_top(ctx) == 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "accept 1 arguments at least");
    }
    if (duk_get_top(ctx) == 1) {
        duk_push_object(ctx);
    }

    duk_enum(ctx, from_id, DUK_ENUM_OWN_PROPERTIES_ONLY);

    while (duk_next(ctx, -1/*enumid*/, 1/*get value*/)) {
        duk_put_prop(ctx, to_id);
    }

    duk_pop(ctx);//pop [enum]

    return 1;
}

// fs.open(fn, mode);
// fs.open(fn);         --default read mode
int _fs_open(duk_context* ctx)
{
    FILE* fp;
    const char* fn;
    const char* mode;

    //check parameters
    if (duk_get_top(ctx) == 2) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }
        if (!duk_is_string(ctx, 1)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file mode must be string");
        }
        mode = duk_get_string(ctx, 1);

    } else if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }
        mode = "r";

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }

    duk_dup(ctx, 0);//copy to top(-1)
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, -1, "UTF-8");
    pylib_encode(ctx, -1, DUKLIB_DEFAULT_SYSENCODING);
#endif
    fn = duk_get_string(ctx, -1);

    //open file
    if ((fp = fopen(fn, mode)) == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "fopen error.");
    }

    ////////////////////////////////////////////////////////
    //generate return object
    duk_push_object(ctx);

    duk_push_string(ctx, "__cobject_file__");
    duk_put_prop_string(ctx, -2, "__type__");

    //remember parameters
    fn = duk_get_string(ctx, 0);
    duk_push_string(ctx, fn);
    duk_put_prop_string(ctx, -2, "__file__");

    duk_push_string(ctx, mode);
    duk_put_prop_string(ctx, -2, "__mode__");

    duk_push_pointer(ctx, fp);
    duk_put_prop_string(ctx, -2, "__handler__");

    //registers c function
    pylib_put_c_method(ctx, "close", _fileobj_close);
    pylib_put_c_method(ctx, "read",  _fileobj_read);
    pylib_put_c_method(ctx, "write", _fileobj_write);

    return 1;
}

// file_put_content(fn, data)
int _fs_file_put_content(duk_context* ctx)
{
    FILE* fp;
    const char* fn;
    const char* s;
    size_t len;
    int bytes_wrote;

    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
    }
    if (!duk_is_string(ctx, 1) && !duk_is_buffer(ctx, 1)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string or buffer");
    }
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, DUKLIB_DEFAULT_SYSENCODING);
#endif
    fn = duk_get_string(ctx, 0);

    if (duk_is_string(ctx, 1)) {
        s = duk_get_lstring(ctx, 1, &len);
    } else {
        s = (const char*)duk_get_buffer(ctx, 1, &len);
    }

    if ((fp = fopen(fn, "wb")) == NULL) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "can't open file");
    }

    bytes_wrote = fwrite(s, 1, len, fp);
    fclose(fp);
    fp = NULL;

    duk_push_int(ctx, bytes_wrote);

    return 1;
}

// file_get_content(fn)
int _fs_file_get_content(duk_context* ctx)
{
    FILE* fp;
    const char* fn;
    int len;
    char* p = NULL;

    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
    }
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, DUKLIB_DEFAULT_SYSENCODING);
#endif
    fn = duk_get_string(ctx, 0);

    if ((fp = fopen(fn, "rb")) != NULL) {
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        p = (char*)duk_alloc_raw(ctx, len + 1);
        memset(p, 0, len + 1);

        fread(p, 1, len, fp);

        fclose(fp);
        fp = NULL;
    }

    if (p != NULL) {
        duk_push_lstring(ctx, p, len);
        duk_free_raw(ctx, p);
    } else {
        duk_push_null(ctx);
    }

    return 1;//one string
}

static int _ps_open(duk_context* ctx)
{
    subprocess_t* ps;
    int length;
    char** args;
    int i;
    const char* s_ptr;
    duk_size_t s_len;

    if (duk_get_top(ctx) == 0) {
        duk_error(ctx, DUK_ERR_RANGE_ERROR, "one parameter at least!");
    }

    if (duk_is_array(ctx, 0)) {
        duk_get_prop_string(ctx, -1, "length");
        length = duk_get_int(ctx, -1);
        if ((duk_is_undefined(ctx, -1)) || (length <= 0)) {
            duk_error(ctx, DUK_ERR_RANGE_ERROR, "Array.length is zero or invalid!");
        }
        duk_pop(ctx);

        args = (char**)duk_alloc_raw(ctx, sizeof(char*) * (length + 1));
        memset(args, 0, sizeof(char*) * (length + 1));

        for (i = 0;i < length;i++) {
            duk_get_prop_index(ctx, -1, i);
            s_ptr = duk_get_lstring(ctx, -1, &s_len);
            args[i] = (char*)duk_alloc_raw(ctx, sizeof(char) * (s_len + 1));
            memset(args[i], 0, sizeof(char) * (s_len + 1));
            memcpy(args[i], s_ptr, s_len);
            duk_pop(ctx);
        }

    } else {
        length = duk_get_top(ctx);

        args = (char**)duk_alloc_raw(ctx, sizeof(char*) * (length + 1));
        memset(args, 0, sizeof(char*) * (length + 1));

        for (i = 0;i < length;i++) {
            s_ptr = duk_get_lstring(ctx, i, &s_len);
            args[i] = (char*)duk_alloc_raw(ctx, sizeof(char) * (s_len + 1));
            memset(args[i], 0, sizeof(char) * (s_len + 1));
            memcpy(args[i], s_ptr, s_len);
            duk_pop(ctx);
        }
    }

    duk_set_top(ctx, 0);//clean all parameters

    ps = psopen(args[0], args);
    if (ps == NULL) {
        for (i = 0;i < length;i++) {
            duk_free_raw(ctx, args[i]);
        }
        duk_free_raw(ctx, args);

        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "native psopen fail!");
    }

    ////////////////////////////////////////////////////////
    //build return object
    duk_push_object(ctx);

    duk_push_pointer(ctx, ps);
    duk_put_prop_string(ctx, -2, "__handler__");

    duk_push_int(ctx, ps->pid);
    duk_put_prop_string(ctx, -2, "pid");

    duk_push_undefined(ctx);
    duk_put_prop_string(ctx, -2, "returncode");

    duk_push_object(ctx);
    duk_push_pointer(ctx, (void*)ps->stdin_fileno);
    duk_put_prop_string(ctx, -2, "__handler__");
    pylib_put_c_method(ctx, "write", _psobj_stdin_write);
    duk_put_prop_string(ctx, -2, "stdin");

    duk_push_object(ctx);
    duk_push_pointer(ctx, (void*)ps->stdout_fileno);
    duk_put_prop_string(ctx, -2, "__handler__");
    pylib_put_c_method(ctx, "read", _psobj_stdout_read);
    duk_put_prop_string(ctx, -2, "stdout");

    pylib_put_c_method(ctx, "wait", _psobj_wait);
    pylib_put_c_method(ctx, "close", _psobj_close);

    duk_push_c_function(ctx, _psobj_finalizer, 1);
    duk_set_finalizer(ctx, -2);

    duk_push_array(ctx);
    for (i = 0;i < length;i++) {
        duk_push_string(ctx, args[i]);
        duk_put_prop_index(ctx, -2, i);
    }
    duk_put_prop_string(ctx, -2, "cmd");

    for (i = 0;i < length;i++) {
        duk_free_raw(ctx, args[i]);
    }
    duk_free_raw(ctx, args);

    return 1;
}

int _os_stat(duk_context* ctx)
{
    const char* fn;
    stat_t st;

    if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }

#ifdef DUKLIB_DEFAULT_SYSENCODING    
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, DUKLIB_DEFAULT_SYSENCODING);
#endif
    fn = duk_get_string(ctx, 0);

    if (stat(fn, &st) != 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "file not exist?");
    }

    duk_push_object(ctx);

    duk_push_string(ctx, "stat_result");
    duk_put_prop_string(ctx, -2, "name");

    pylib_put_c_method(ctx, "toString", _statobj_tostring);

    _helper_pack_statobj(ctx, &st, -1);

    return 1;
}

int _os_getcwd(duk_context* ctx)
{
    char* currdir;

    currdir = getcwd(NULL, 0);
    if (currdir == NULL) {
        duk_push_undefined(ctx);
        return 1;
    }

    duk_push_string(ctx, currdir);
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, -1, DUKLIB_DEFAULT_SYSENCODING);
    pylib_encode(ctx, -1, "UTF-8");
#endif

    free(currdir);

    return 1;
}

int _ospath_isdir(duk_context* ctx)
{
    const char* fn;
    stat_t st;

    if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, "GBK");
#endif
    fn = duk_get_string(ctx, 0);

    if (stat(fn, &st) != 0) {
        duk_push_false(ctx);
        return 1;
    }

    if (st.st_mode & S_IFDIR) {
        duk_push_true(ctx);
    } else {
        duk_push_false(ctx);
    }

    return 1;
}

int _ospath_isfile(duk_context* ctx)
{
    const char* fn;
    stat_t st;

    if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, "GBK");
#endif
    fn = duk_get_string(ctx, 0);

    if (stat(fn, &st) != 0) {
        duk_push_false(ctx);
        return 1;
    }

    if (st.st_mode & S_IFDIR) {
        duk_push_false(ctx);
    } else {
        duk_push_true(ctx);
    }

    return 1;
}

int _ospath_exists(duk_context* ctx)
{
    const char* fn;
    stat_t st;

    if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, 0, "UTF-8");
    pylib_encode(ctx, 0, "GBK");
#endif
    fn = duk_get_string(ctx, 0);

    if (stat(fn, &st) != 0) {
        duk_push_false(ctx);
    } else {
        duk_push_true(ctx);
    }

    return 1;
}

int _ospath_join(duk_context* ctx)
{
    int n;
    int i;
    int total_len = 0;
    duk_size_t s_len;
    const char* s;
    char* buf;
    int len;

#ifdef DUK_F_WINDOWS
    char split_char = '\\';
#endif
#ifdef DUK_F_LINUX
    char split_char = '/';
#endif

    n = duk_get_top(ctx);
    for (i = 0;i < n;i++) {
        duk_get_lstring(ctx, i, &s_len);
        total_len += s_len + 1;
    }

    if (total_len <= 0) {
        duk_push_undefined(ctx);
        return 1;
    }

    len = 0;
    buf = (char*)duk_alloc_raw(ctx, total_len+n+1);
    memset(buf, 0, total_len+n+1);

    for (i = 0;i < n;i++) {
        s = duk_get_lstring(ctx, i, &s_len);
        if (s_len <= 0) {
            continue;
        }
        //first variable
        if (len == 0) {
            memcpy(buf, s, s_len);
            len += s_len;
            continue;
        }
        //overwrite previous variables.
        if ((s[0] == '/') || (s[0] == '\\')) {
            memcpy(buf, s, s_len);
            len += s_len;
            continue;
        }

        if ((buf[len-1] != '/') && (buf[len-1] != '\\')) {
            buf[len++] = split_char;
        }
        memcpy(buf+len, s, s_len);
        len += s_len;
    }

    duk_push_string(ctx, buf);
    duk_free_raw(ctx, buf);
    return 1;
}

int _ospath_dirname(duk_context* ctx)
{
    int i;
    duk_size_t n;
    int len_1th;
    const char* fn;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }

    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
    }

    fn = duk_get_lstring(ctx, 0, &n);
    if (n <= 0) {
        duk_push_string(ctx, "");
        return 1;
    }

    for (i = n-1;i >= 0;i--) {
        if ((fn[i] == '/') || (fn[i] == '\\')) {
            break;
        }
    }

    if (i < 0) {
        len_1th = n;
    } else {
        len_1th = i;
    }

    if (len_1th > 0) {
        duk_push_lstring(ctx, fn, len_1th);
    } else {
        duk_push_string(ctx, "");
    }

    return 1;
}

int _ospath_split(duk_context* ctx)
{
    int i;
    duk_size_t n;
    int len_1th;
    int len_2nd;
    const char* fn;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }

    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
    }

    fn = duk_get_lstring(ctx, 0, &n);
    if (n <= 0) {
        duk_push_array(ctx);
        duk_push_string(ctx, "");
        duk_put_prop_index(ctx, -2, 0);
        duk_push_string(ctx, "");
        duk_put_prop_index(ctx, -2, 0);
        return 1;
    }

    for (i = n-1;i >= 0;i--) {
        if ((fn[i] == '/') || (fn[i] == '\\')) {
            break;
        }
    }

    if (i < 0) {
        len_1th = n;
        len_2nd = 0;
    } else {
        len_1th = i;
        len_2nd = n-i-1;
    }

    duk_push_array(ctx);

    if (len_1th > 0) {
        duk_push_lstring(ctx, fn, len_1th);
    } else {
        duk_push_string(ctx, "");
    }
    duk_put_prop_index(ctx, -2, 0);

    if (len_2nd > 0) {
        duk_push_lstring(ctx, fn + len_1th + 1, len_2nd);
    } else {
        duk_push_string(ctx, "");
    }
    duk_put_prop_index(ctx, -2, 1);

    return 1;
}

int _ospath_splitext(duk_context* ctx)
{
    int i;
    duk_size_t n;
    int len_1th;
    int len_2nd;
    const char* fn;

    if (duk_get_top(ctx) == 1) {
        if (!duk_is_string(ctx, 0)) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "file name must be string");
        }
        fn = duk_get_string(ctx, 0);

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing parameters");
    }

    fn = duk_get_lstring(ctx, 0, &n);
    if (n <= 0) {
        duk_push_array(ctx);
        duk_push_string(ctx, "");
        duk_put_prop_index(ctx, -2, 0);
        duk_push_string(ctx, "");
        duk_put_prop_index(ctx, -2, 0);
        return 1;
    }

    for (i = n-1;i >= 0;i--) {
        if (fn[i] == '.') {
            break;
        }
    }

    if (i < 0) {
        len_1th = n;
        len_2nd = 0;
    } else {
        len_1th = i;
        len_2nd = n-i-1;
    }

    duk_push_array(ctx);

    if (len_1th > 0) {
        duk_push_lstring(ctx, fn, len_1th);
    } else {
        duk_push_string(ctx, "");
    }
    duk_put_prop_index(ctx, -2, 0);

    if (len_2nd > 0) {
        duk_push_lstring(ctx, fn + len_1th + 1, len_2nd);
    } else {
        duk_push_string(ctx, "");
    }
    duk_put_prop_index(ctx, -2, 1);

    return 1;
}

int _ospath_normpath(duk_context* ctx)
{
    const char* s;
    char* p;
    duk_size_t n;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept one string");
    }
    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }

    s = duk_get_lstring(ctx, 0, &n);
    p = (char*)duk_alloc_raw(ctx, n);
    memcpy(p, s, n);

    n = _helper_path_normpath(ctx, p, n);
    duk_push_lstring(ctx, p, n);
    duk_free_raw(ctx, p);

    return 1;
}

int _ospath_abspath(duk_context* ctx)
{
    const char* s;
    duk_size_t len;

    if (duk_get_top(ctx) != 1) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept one string");
    }
    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }

    s = duk_get_lstring(ctx, 0, &len);
    if ((s[0] == '/') || (s[0] == '\\') ||
        ((len >= 2) && isalpha(s[0]) && (s[1] == ':')))
    {//absolute path already, just need normal path format.

    } else {
        duk_push_c_function(ctx, _os_getcwd, 0);
        duk_call(ctx, 0);
        if (duk_is_undefined(ctx, -1)) {
            return 1;
        }

        duk_push_c_function(ctx, _ospath_join, 2);
        duk_dup(ctx, -2);
        duk_dup(ctx, 0);
        duk_call(ctx, 2);

        duk_replace(ctx, 0);
        duk_pop(ctx);
    }

    duk_push_c_function(ctx, _ospath_normpath, 1);
    duk_dup(ctx, 0);
    duk_call(ctx, 1);

    return 1;
}

int _time_time(duk_context* ctx)
{
    time_t t;

    time(&t);

    duk_push_uint(ctx, (unsigned int)t);

    return 1;
}

int _time_localtime(duk_context* ctx)
{
    time_t t;
    struct tm ts;

    if ((duk_get_top(ctx) == 1) && !duk_is_number(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept integer");
    }

    if (duk_get_top(ctx) == 1) {
        t = duk_to_uint(ctx, 0);
    } else {
        time(&t);
    }

    ts = *localtime(&t);

    duk_push_object(ctx);

    duk_push_string(ctx, "struct_time");
    duk_put_prop_string(ctx, -2, "name");

    pylib_put_c_method(ctx, "toString", _timeobj_tostring);

    _helper_pack_timeobj(ctx, &ts, -1);

    return 1;
} 

int _time_gmtime(duk_context* ctx)
{
    time_t t;
    struct tm ts;

    if ((duk_get_top(ctx) == 1) && !duk_is_number(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept integer");
    }

    if (duk_get_top(ctx) == 1) {
        t = duk_to_uint(ctx, 0);
    } else {
        time(&t);
    }

    ts = *gmtime(&t);

    duk_push_object(ctx);

    duk_push_string(ctx, "struct_time");
    duk_put_prop_string(ctx, -2, "name");

    pylib_put_c_method(ctx, "toString", _timeobj_tostring);

    _helper_pack_timeobj(ctx, &ts, -1);

    return 1;
} 

int _time_asctime(duk_context* ctx)
{
    time_t t;
    struct tm ts;
    const char* s;
    char buf[100];

    if (duk_get_top(ctx) == 0) {
        time(&t);
        ts = *localtime(&t);

    } else if ((duk_get_top(ctx) == 1) && duk_is_object(ctx, 0)) {
        duk_get_prop_string(ctx, 0, "name");
        s = duk_to_string(ctx, -1);
        if (strcmp(s, "struct_time") == 0) {
            _helper_unpack_timeobj(ctx, &ts, 0);
            duk_pop(ctx);
        } else {
            duk_error(ctx, DUK_ERR_TYPE_ERROR,
                    "second parameter only accept time.struct_time");
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "parameter error");
    }

    //Sat May 20 15:21:51 2000
    strftime(buf, 100, "%a %b %d %H:%M:%S %Y", &ts);

    duk_push_string(ctx, buf);

    return 1;
} 

int _time_ctime(duk_context* ctx)
{
    time_t t;
    struct tm ts;
    char buf[100];

    if ((duk_get_top(ctx) == 1) && !duk_is_number(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept integer");
    }

    if (duk_get_top(ctx) == 1) {
        t = duk_to_uint(ctx, 0);
    } else {
        time(&t);
    }

    ts = *localtime(&t);
    //Sat May 20 15:21:51 2000
    strftime(buf, 100, "%a %b %d %H:%M:%S %Y", &ts);

    duk_push_string(ctx, buf);

    return 1;
} 

int _time_strftime(duk_context* ctx)
{
    time_t t;
    struct tm ts;

    const char* s;
    const char* fmt;
    int len;
    char* out;

    int success = 0;
    int ntries = 3;

    if (duk_get_top(ctx) == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing format string");

    } else if (duk_get_top(ctx) == 1) {
        time(&t);
        ts = *localtime(&t);

    } else if (duk_get_top(ctx) == 2) {
        if (duk_is_number(ctx, 1)) {
            t = duk_to_uint(ctx, 1);
            ts = *localtime(&t);
        } else {
            duk_get_prop_string(ctx, 1, "name");
            s = duk_to_string(ctx, -1);
            if (strcmp(s, "struct_time") == 0) {
                _helper_unpack_timeobj(ctx, &ts, 1);
                duk_pop(ctx);
            } else {
                duk_error(ctx, DUK_ERR_TYPE_ERROR,
                        "second parameter only accept time or time.struct_time");
            }
        }

    } else {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "parameter only accept time or time.struct_time");
    }

    if (!duk_is_string(ctx, 0)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }

    fmt = duk_to_string(ctx, 0);
    len = strlen(fmt) + 100;
    out = (char*)duk_alloc_raw(ctx, len);

    while (ntries--) {
        success = strftime(out, len, fmt, &ts);
        if (success) {
            break;
        }

        len = len * 2;
        out = (char*)duk_realloc_raw(ctx, out, len);
    }

    duk_push_lstring(ctx, out, strlen(out));
    duk_free_raw(ctx, out);

    return 1;
}

char* pylib_iconv(duk_context* ctx, const char* instr, int inlen, int* outlen,
        const char* inenc, const char* outenc)
{
    char* inbuf;
    char* outbuf;
    size_t bytes_in;
    size_t bytes_left;

    char* pbuf;
    int nbuf;

    iconv_t ctl;

    ctl = iconv_open(outenc, inenc);
    if (ctl <= 0) {
        return NULL;
    }

    //prepare out buffer
    nbuf = inlen * 8;
    pbuf = duk_alloc_raw(ctx, nbuf);
    memset(pbuf, 0, nbuf);

    //convert, n_mbcs means bytes left.
    inbuf  = (char*)instr;
    outbuf = pbuf;
    bytes_in   = inlen;
    bytes_left = nbuf;

    if (iconv(ctl, &inbuf, &bytes_in, &outbuf, &bytes_left) != 0) {
        return 0;
    }

    if (outlen != NULL) {
        *outlen = nbuf - bytes_left;
    }

    iconv_close(ctl);
    ctl = 0;

    return pbuf;
}

//Tips: encoding of fn must convert before used!
//make sure is DUKLIB_DEFAULT_SYSENCODING
int pylib_include(duk_context* ctx, const char* fn)
{
    duk_push_c_function(ctx, _include, 1);
    duk_push_string(ctx, fn);
#ifdef DUKLIB_DEFAULT_SYSENCODING
    pylib_decode(ctx, -1, DUKLIB_DEFAULT_SYSENCODING);
    pylib_encode(ctx, -1, "UTF-8");
#endif

    if (duk_pcall(ctx, 1) != 0) {
        duk_safe_call(ctx, _helper_get_stack_raw, 1 /*nargs*/, 1 /*nrets*/);
        fprintf(stderr, "%s\n", duk_safe_to_string(ctx, -1));
        fflush(stderr);
        duk_pop(ctx);

        return 0;
    }
    duk_pop(ctx);

    return 1;
}

int pylib_eval_string(duk_context* ctx, const char* s)
{
    if (duk_peval_string(ctx, s) != 0) {
        duk_safe_call(ctx, _helper_get_stack_raw, 1 /*nargs*/, 1 /*nrets*/);

        fprintf(stderr, "%s\n", duk_safe_to_string(ctx, -1));
        fflush(stderr);

        return 0;
    }

    return 1;
}

int pylib_put_args(duk_context* ctx, int argc, char* argv[])
{
    int obj_id = 0;
    int i = 0;
    int idx = 0;

    duk_push_global_object(ctx);
    duk_get_prop_string(ctx, -1, "Modules");
    if (duk_is_undefined(ctx, -1)) {
        return 0;
    }

    duk_get_prop_string(ctx, -1, "sys");
    if (duk_is_undefined(ctx, -1)) {
        return 0;
    }

    obj_id = duk_push_array(ctx);
    for (i = 1;i < argc;i++) {
        duk_push_lstring(ctx, argv[i], strlen(argv[i]));
#ifdef DUKLIB_DEFAULT_SYSENCODING
        pylib_decode(ctx, -1, DUKLIB_DEFAULT_SYSENCODING);
        pylib_encode(ctx, -1, "UTF-8");
#endif
        duk_put_prop_index(ctx, obj_id, idx++);
    }
    duk_put_prop_string(ctx, -2, "args");

    duk_pop(ctx);//pop 'sys'
    duk_pop(ctx);//pop 'Modules'
    duk_pop(ctx);//pop 'global'

    return argc-1;
}

//2 step:
//
//first : set function_entry.name  = function_name
//second: set global.function_name = function_entry
//must push object to stack first!
int pylib_put_c_method(duk_context* ctx,
        const char* function_name,
        duk_c_function function_entry)
{
    char* p;
    int n;

    duk_push_c_function(ctx, function_entry, DUK_VARARGS);

    n = strlen(function_name);
    p = duk_alloc_raw(ctx, n + 10 + 1);
    memset(p, 0, n + 10 + 1);
    memcpy(p, "__stdlib__", 10);
    memcpy(p+10, function_name, n);
    duk_push_string(ctx, p);
    duk_free_raw(ctx, p);
    duk_put_prop_string(ctx, -2, "name");

    duk_put_prop_string(ctx, -2, function_name);

    return 0;
}

int pylib_put_c_function(duk_context* ctx,
        const char* function_name,
        duk_c_function function_entry)
{
    duk_push_global_object(ctx);
    pylib_put_c_method(ctx, function_name, function_entry);
    duk_pop(ctx);

    return 0;
}

const char* pylib_type_name(duk_context* ctx, duk_idx_t index)
{
    int t;

    t = duk_get_type(ctx, index);

    switch(t) {
        case DUK_TYPE_NONE:
            return "none";
        case DUK_TYPE_UNDEFINED:
            return "undefined";
        case DUK_TYPE_NULL:
            return "null";
        case DUK_TYPE_BOOLEAN:
            return "boolean";
        case DUK_TYPE_NUMBER:
            return "number";
        case DUK_TYPE_STRING:
            return "string";
        case DUK_TYPE_OBJECT:
            return "object";
        case DUK_TYPE_BUFFER:
            return "buffer";
        case DUK_TYPE_POINTER:
            return "pointer";
        default:
            return "error";
    }
}

int pylib_encode(duk_context* ctx, int idx, const char* encoding)
{
    const char* s_ucs2;
    size_t n_ucs2;
    char* s_mbcs;
    size_t n_mbcs;

    char* inbuf;
    char* outbuf;
    size_t bytes_in;
    size_t bytes_left;

    iconv_t ctl;

    if (idx < 0) {
        idx = duk_get_top(ctx) + idx;
    }

    if (!duk_is_string(ctx, idx)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }

    if (strlen(encoding) == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not accept empty string");
    }

    ctl = iconv_open(encoding, "UCS-2LE");
    if (ctl <= 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }

    //get original string
    s_ucs2 = duk_get_lstring(ctx, idx, &n_ucs2);//not-Null string!

    //prepare out buffer
    n_mbcs = n_ucs2 * 8;
    s_mbcs = duk_alloc_raw(ctx, n_mbcs);

    //convert, n_mbcs means bytes left.
    inbuf  = (char*)s_ucs2;
    outbuf = s_mbcs;
    bytes_in   = n_ucs2;
    bytes_left = n_mbcs;

    if (iconv(ctl, &inbuf, &bytes_in, &outbuf, &bytes_left) != 0) {
        return 0;
    }
    n_mbcs = n_mbcs - bytes_left;

    duk_push_lstring(ctx, s_mbcs, n_mbcs);
    duk_insert(ctx, idx);
    duk_remove(ctx, idx+1);

    //cleanup
    duk_free_raw(ctx, s_mbcs);
    s_mbcs = NULL;

    iconv_close(ctl);
    ctl = 0;

    return 1;
}

int pylib_decode(duk_context* ctx, int idx, const char* encoding)
{
    const char* s_mbcs;
    size_t n_mbcs;
    char* s_ucs2;
    size_t n_ucs2;

    char* inbuf;
    char* outbuf;
    size_t bytes_in;
    size_t bytes_left;

    iconv_t ctl;

    if (idx < 0) {
        idx = duk_get_top(ctx) + idx;
    }

    if (!duk_is_string(ctx, idx)) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "only accept string");
    }

    if (strlen(encoding) == 0) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not accept empty string");
    }

    ctl = iconv_open("UCS-2LE", encoding);
    if (ctl <= 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "iconv error");
    }

    //get original string
    s_mbcs = duk_get_lstring(ctx, idx, &n_mbcs);//not-Null string!

    //prepare out buffer
    n_ucs2 = n_mbcs * 8;
    s_ucs2 = duk_alloc_raw(ctx, n_ucs2);

    //convert, n_ucs2 means bytes left.
    inbuf  = (char*)s_mbcs;
    outbuf = s_ucs2;
    bytes_in   = n_mbcs;
    bytes_left = n_ucs2;

    if (iconv(ctl, &inbuf, &bytes_in, &outbuf, &bytes_left) != 0) {
        return 0;
    }
    n_ucs2 = n_ucs2 - bytes_left;

    duk_push_lstring(ctx, s_ucs2, n_ucs2);
    duk_insert(ctx, idx);
    duk_remove(ctx, idx+1);

    //cleanup
    duk_free_raw(ctx, s_ucs2);
    s_ucs2 = NULL;

    iconv_close(ctl);
    ctl = 0;

    return 1;
}

void _dukopen_buildin_extend(duk_context* ctx)
{
    duk_push_global_object(ctx);

    pylib_put_c_method(ctx, "ord",                 _ord);
    pylib_put_c_method(ctx, "chr",                 _chr);
    pylib_put_c_method(ctx, "hex",                 _hex);
    pylib_put_c_method(ctx, "globals",             _globals);
    pylib_put_c_method(ctx, "dir",                 _dir);
    pylib_put_c_method(ctx, "include",             _include);
    pylib_put_c_method(ctx, "deepcopy1",           _deepcopy1);

    duk_pop(ctx);
}

void _push_traces_obj(duk_context* ctx)
{
    duk_push_global_object(ctx);
    duk_push_undefined(ctx);
    duk_put_prop_string(ctx, -2, "__file__");
    duk_pop(ctx);

    duk_push_array(ctx);
}

void _push_fs_obj(duk_context* ctx)
{
    duk_push_object(ctx);
    pylib_put_c_method(ctx, "open",                _fs_open);
    pylib_put_c_method(ctx, "file_get_content",    _fs_file_get_content);
    pylib_put_c_method(ctx, "file_put_content",    _fs_file_put_content);
}

void _push_os_path_obj(duk_context* ctx)
{
    duk_push_object(ctx);
    pylib_put_c_method(ctx, "isdir",               _ospath_isdir);
    pylib_put_c_method(ctx, "isfile",              _ospath_isfile);
    pylib_put_c_method(ctx, "exists",              _ospath_exists);
    pylib_put_c_method(ctx, "join",                _ospath_join);
    pylib_put_c_method(ctx, "dirname",             _ospath_dirname);
    pylib_put_c_method(ctx, "split",               _ospath_split);
    pylib_put_c_method(ctx, "splitext",            _ospath_splitext);
    pylib_put_c_method(ctx, "normpath",            _ospath_normpath);
    pylib_put_c_method(ctx, "abspath",             _ospath_abspath);
}

void _push_os_obj(duk_context* ctx)
{
    duk_push_object(ctx);

    pylib_put_c_method(ctx, "getcwd",              _os_getcwd);
    pylib_put_c_method(ctx, "stat",                _os_stat);

#ifdef DUK_F_WINDOWS
    duk_push_string(ctx, "nt");
#else
    duk_push_string(ctx, "posix");
#endif
    duk_put_prop_string(ctx, -2, "name");
}

void _push_time_obj(duk_context* ctx)
{
    duk_push_object(ctx);
    pylib_put_c_method(ctx, "time",                _time_time);
    pylib_put_c_method(ctx, "localtime",           _time_localtime);
    pylib_put_c_method(ctx, "gmtime",              _time_gmtime);
    pylib_put_c_method(ctx, "asctime",             _time_asctime);
    pylib_put_c_method(ctx, "ctime",               _time_ctime);
    pylib_put_c_method(ctx, "strftime",            _time_strftime);
}

void _push_sys_obj(duk_context* ctx)
{
    duk_push_object(ctx);
    duk_push_undefined(ctx);
    duk_put_prop_string(ctx, -2, "args");
}

void _push_subprocess_obj(duk_context* ctx)
{
    duk_push_object(ctx);
    pylib_put_c_method(ctx, "open", _ps_open);
}

void _set_modsearch(duk_context* ctx)
{
    const char script[] = 
        "Duktape.modSearch = function (id, require, exports, module) {\r\n"
        "    var obj = Modules[id];\r\n"
        "    if (obj) {\r\n"
        "        deepcopy1(obj, exports);\r\n"
        "        return;\r\n"
        "\r\n"
        "    } else {\r\n"
        "        //try load script\r\n"
        "        var os         = Modules.os;\r\n"
        "        var fs         = Modules.fs;\r\n"
        "        var path       = os.path;\r\n"
        "        var dirname    = path.dirname(__file__);\r\n"
        "        var fn         = path.join(dirname, id);\r\n"
        "        var absfn      = path.abspath(fn);\r\n"
        "        if (!path.isfile(absfn)) {\r\n"
        "            dirname    = os.getcwd();\r\n"
        "            fn         = path.join(dirname, id);\r\n"
        "            absfn      = path.abspath(fn);\r\n"
        "            if (!path.isfile(absfn)) {\r\n"
        "                throw new Error('module not found: ' + id);\r\n"
        "            }\r\n"
        "        }\r\n"
        "\r\n"
        "        return fs.file_get_content(absfn);\r\n"
        "    }\r\n"
        "\r\n"
        "    throw new Error('module not found: ' + id);\r\n"
        "}\r\n";

    duk_eval_string_noresult(ctx, script);
}

int dukopen_pylib(duk_context* ctx)
{
    _dukopen_buildin_extend(ctx);

    ////////////////////////////////////////////////////////
    duk_push_global_object(ctx);
    duk_push_object(ctx);


    _push_traces_obj(ctx);
    duk_put_prop_string(ctx, -2, "traces");

    _push_fs_obj(ctx);
    duk_put_prop_string(ctx, -2, "fs");

    _push_os_obj(ctx);
    _push_os_path_obj(ctx);
    duk_put_prop_string(ctx, -2, "path");
    duk_put_prop_string(ctx, -2, "os");

    _push_os_path_obj(ctx);
    duk_put_prop_string(ctx, -2, "os.path");

    _push_time_obj(ctx);
    duk_put_prop_string(ctx, -2, "time");

    _push_sys_obj(ctx);
    duk_put_prop_string(ctx, -2, "sys");

    _push_subprocess_obj(ctx);
    duk_put_prop_string(ctx, -2, "_subprocess");


    duk_put_prop_string(ctx, -2, "Modules");
    duk_pop(ctx);//pop [global]

    ////////////////////////////////////////////////////////
    _set_modsearch(ctx);

    duk_eval_string(ctx, "String.prototype");
    pylib_put_c_method(ctx, "encode", _strobj_encode);
    pylib_put_c_method(ctx, "decode", _strobj_decode);
    duk_pop(ctx);

    //duk_eval_string_noresult(ctx,
    //        "subprocess.CalledProcessError = "
    //        "function (returncode, cmd, output) {"
    //        "    this.returncode = returncode;"
    //        "    this.cmd        = cmd;"
    //        "    this.output     = output;"
    //        "}"
    //);

    return 0;
}



