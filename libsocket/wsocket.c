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
 *
 * this code copy and modify from luasocket, which license is MIT:)
 *  @ref    luasocket/libsocket.h
 *  @ref    luasocket/usocket.c
 *  @ref    luasocket/wsocket.c
 *
 *
 * LuaSocket 2.0.2 license
 * Copyright ?2004-2007 Diego Nehab
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * @ref winerror.h
 *
 * #define WSABASEERR                  10000
 * #define WSAEINTR                    (WSABASEERR + 4)
 * #define WSAEBADF                    (WSABASEERR + 9)
 * #define WSAEACCES                   (WSABASEERR + 13)
 * #define WSAEFAULT                   (WSABASEERR + 14)
 * #define WSAEINVAL                   (WSABASEERR + 22)
 * #define WSAEMFILE                   (WSABASEERR + 24)
 * #define WSAEWOULDBLOCK              (WSABASEERR + 35)
 * #define WSAEINPROGRESS              (WSABASEERR + 36)
 * #define WSAEALREADY                 (WSABASEERR + 37)
 * #define WSAENOTSOCK                 (WSABASEERR + 38)
 * #define WSAEDESTADDRREQ             (WSABASEERR + 39)
 * #define WSAEMSGSIZE                 (WSABASEERR + 40)
 * #define WSAEPROTOTYPE               (WSABASEERR + 41)
 * #define WSAENOPROTOOPT              (WSABASEERR + 42)
 * #define WSAEPROTONOSUPPORT          (WSABASEERR + 43)
 * #define WSAESOCKTNOSUPPORT          (WSABASEERR + 44)
 * #define WSAEOPNOTSUPP               (WSABASEERR + 45)
 * #define WSAEPFNOSUPPORT             (WSABASEERR + 46)
 * #define WSAEAFNOSUPPORT             (WSABASEERR + 47)
 * #define WSAEADDRINUSE               (WSABASEERR + 48)
 * #define WSAEADDRNOTAVAIL            (WSABASEERR + 49)
 * #define WSAENETDOWN                 (WSABASEERR + 50)
 * #define WSAENETUNREACH              (WSABASEERR + 51)
 * #define WSAENETRESET                (WSABASEERR + 52)
 * #define WSAECONNABORTED             (WSABASEERR + 53)
 * #define WSAECONNRESET               (WSABASEERR + 54)
 * #define WSAENOBUFS                  (WSABASEERR + 55)
 * #define WSAEISCONN                  (WSABASEERR + 56)
 * #define WSAENOTCONN                 (WSABASEERR + 57)
 * #define WSAESHUTDOWN                (WSABASEERR + 58)
 * #define WSAETOOMANYREFS             (WSABASEERR + 59)
 * #define WSAETIMEDOUT                (WSABASEERR + 60)
 * #define WSAECONNREFUSED             (WSABASEERR + 61)
 * #define WSAELOOP                    (WSABASEERR + 62)
 * #define WSAENAMETOOLONG             (WSABASEERR + 63)
 * #define WSAEHOSTDOWN                (WSABASEERR + 64)
 * #define WSAEHOSTUNREACH             (WSABASEERR + 65)
 * #define WSAENOTEMPTY                (WSABASEERR + 66)
 * #define WSAEPROCLIM                 (WSABASEERR + 67)
 * #define WSAEUSERS                   (WSABASEERR + 68)
 * #define WSAEDQUOT                   (WSABASEERR + 69)
 * #define WSAESTALE                   (WSABASEERR + 70)
 * #define WSAEREMOTE                  (WSABASEERR + 71)
 * #define WSASYSNOTREADY              (WSABASEERR + 91)
 * #define WSAVERNOTSUPPORTED          (WSABASEERR + 92)
 * #define WSANOTINITIALISED           (WSABASEERR + 93)
 * #define WSAEDISCON                  (WSABASEERR + 101)
 * #define WSAENOMORE                  (WSABASEERR + 102)
 * #define WSAECANCELLED               (WSABASEERR + 103)
 * #define WSAEINVALIDPROCTABLE        (WSABASEERR + 104)
 * #define WSAEINVALIDPROVIDER         (WSABASEERR + 105)
 * #define WSAEPROVIDERFAILEDINIT      (WSABASEERR + 106)
 * #define WSASYSCALLFAILURE           (WSABASEERR + 107)
 * #define WSASERVICE_NOT_FOUND        (WSABASEERR + 108)
 * #define WSATYPE_NOT_FOUND           (WSABASEERR + 109)
 * #define WSA_E_NO_MORE               (WSABASEERR + 110)
 * #define WSA_E_CANCELLED             (WSABASEERR + 111)
 * #define WSAEREFUSED                 (WSABASEERR + 112)
 * 
 * #define WSAHOST_NOT_FOUND           (WSABASEERR + 1001)
 * #define WSATRY_AGAIN                (WSABASEERR + 1002)
 * #define WSANO_RECOVERY              (WSABASEERR + 1003)
 * #define WSANO_DATA                  (WSABASEERR + 1004)
 * #define WSA_QOS_RECEIVERS           (WSABASEERR + 1005)
 * #define WSA_QOS_SENDERS             (WSABASEERR + 1006)
 * #define WSA_QOS_NO_SENDERS          (WSABASEERR + 1007)
 * #define WSA_QOS_NO_RECEIVERS        (WSABASEERR + 1008)
 * #define WSA_QOS_REQUEST_CONFIRMED   (WSABASEERR + 1009)
 * #define WSA_QOS_ADMISSION_FAILURE   (WSABASEERR + 1010)
 * #define WSA_QOS_POLICY_FAILURE      (WSABASEERR + 1011)
 * #define WSA_QOS_BAD_STYLE           (WSABASEERR + 1012)
 * #define WSA_QOS_BAD_OBJECT          (WSABASEERR + 1013)
 * #define WSA_QOS_TRAFFIC_CTRL_ERROR  (WSABASEERR + 1014)
 * #define WSA_QOS_GENERIC_ERROR       (WSABASEERR + 1015)
 * #define WSA_QOS_ESERVICETYPE        (WSABASEERR + 1016)
 * #define WSA_QOS_EFLOWSPEC           (WSABASEERR + 1017)
 * #define WSA_QOS_EPROVSPECBUF        (WSABASEERR + 1018)
 * #define WSA_QOS_EFILTERSTYLE        (WSABASEERR + 1019)
 * #define WSA_QOS_EFILTERTYPE         (WSABASEERR + 1020)
 * #define WSA_QOS_EFILTERCOUNT        (WSABASEERR + 1021)
 * #define WSA_QOS_EOBJLENGTH          (WSABASEERR + 1022)
 * #define WSA_QOS_EFLOWCOUNT          (WSABASEERR + 1023)
 * #define WSA_QOS_EUNKNOWNPSOBJ       (WSABASEERR + 1024)
 * #define WSA_QOS_EUNKOWNPSOBJ        (WSABASEERR + 1024)
 * #define WSA_QOS_EPOLICYOBJ          (WSABASEERR + 1025)
 * #define WSA_QOS_EFLOWDESC           (WSABASEERR + 1026)
 * #define WSA_QOS_EPSFLOWSPEC         (WSABASEERR + 1027)
 * #define WSA_QOS_EPSFILTERSPEC       (WSABASEERR + 1028)
 * #define WSA_QOS_ESDMODEOBJ          (WSABASEERR + 1029)
 * #define WSA_QOS_ESHAPERATEOBJ       (WSABASEERR + 1030)
 * #define WSA_QOS_RESERVED_PETYPE     (WSABASEERR + 1031)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "libsocket.h"

#ifdef TARGET_WINDOWS
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#endif

int socket_waitfd(socket_t* h, int sw, timeout_t tm)
{
    int ret;
    fd_set rfds;
    fd_set wfds;
    fd_set efds;
    fd_set* rp = NULL;
    fd_set* wp = NULL;
    fd_set* ep = NULL;
    struct timeval tv;
    struct timeval* tp = NULL;

    if (tm == 0) {
        return IO_TIMEOUT;
    }

    if (sw & WAITFD_R) { 
        FD_ZERO(&rfds); 
		FD_SET(*h, &rfds);
        rp = &rfds; 
    }

    if (sw & WAITFD_W) {
        FD_ZERO(&wfds);
        FD_SET(*h, &wfds);
        wp = &wfds;
    }

    if (sw & WAITFD_E) {
        FD_ZERO(&efds);
        FD_SET(*h, &efds);
        ep = &efds;
    }

    if (tm >= 0.0) {
        tv.tv_sec = (int) tm;
        tv.tv_usec = (int) ((tm-tv.tv_sec)*1.0e6);
        tp = &tv;
    }

    ret = select(0, rp, wp, ep, tp);

    if (ret == -1) {
        return WSAGetLastError();
    }

    if (ret == 0) {
        return IO_TIMEOUT;
    }

    if (sw == WAITFD_C && FD_ISSET(*h, &efds)) {
        return IO_CLOSED;
    }

    return IO_DONE;
}

int socket_select(socket_t* n, fd_set *rfds, fd_set *wfds, fd_set *efds, timeout_t tm)
{
    struct timeval tv; 

    tv.tv_sec = (int) tm;
    tv.tv_usec = (int) ((tm - tv.tv_sec) * 1.0e6);

    if (n <= 0) {
        Sleep((DWORD) (1000*tm));//Error?
        return 0;
    } else {
        return select(0, rfds, wfds, efds, tm >= 0.0? &tv: NULL);
    }
}

int socket_startup(void)
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0); 
    int err = WSAStartup(wVersionRequested, &wsaData );
    if (err != 0) return 0;
    if ((LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) &&
        (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)) {
        WSACleanup();
        return 0; 
    }
    return 1;
}

int socket_cleanup(void)
{
    WSACleanup();
    return 1;
}

int socket_create(socket_t* h, int domain, int type, int protocol)
{
    *h = socket(domain, type, protocol);
    if (*h == SOCKET_INVALID) {
        return WSAGetLastError();
    }
    return IO_DONE;
}

void socket_destroy(socket_t* h)
{
    if (*h != SOCKET_INVALID) {
        socket_setblocking(h);
        closesocket(*h);
        *h = SOCKET_INVALID;
    }
}

//how:
//  A flag that describes what types of operation will no longer be allowed. 
//  SD_RECEIVE    0
//  SD_SEND       1
//  SD_BOTH       2
void socket_shutdown(socket_t* h, int how)
{
    socket_setblocking(h);
    shutdown(*h, how);
    socket_setnonblocking(h);
}


int socket_connect(socket_t* h, sockaddr *addr, socklen_t len, timeout_t tm)
{
    int err;

    if (*h == SOCKET_INVALID) {
        return IO_CLOSED;
    }

    if (connect(*h, addr, len) == 0) {
        return IO_DONE;
    }

    err = WSAGetLastError();
    if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
        return err;
    }

    /* zero timeout case optimization */
    if (tm == 0) {
        return IO_TIMEOUT;
    }

    /* we wait until something happens */
    err = socket_waitfd(h, WAITFD_C, tm);
    if (err == IO_CLOSED) {
        int len = sizeof(err);

        /* give windows time to set the error (yes, disgusting) */
        Sleep(10);

        /* find out why we failed */
        getsockopt(*h, SOL_SOCKET, SO_ERROR, (char *)&err, &len); 

        /* we KNOW there was an error. if 'why' is 0, we will return
        * "unknown error", but it's not really our fault */
        return (err > 0) ? err: IO_UNKNOWN; 

    } else {
        return err;
    }
}

int socket_bind(socket_t* h, sockaddr *addr, socklen_t len)
{
    int err = IO_DONE;

    socket_setblocking(h);
    if (bind(*h, addr, len) < 0) {
        err = WSAGetLastError();
    }
    socket_setnonblocking(h);

    return err;
}

int socket_listen(socket_t* h, int backlog)
{
    int err = IO_DONE;
    socket_setblocking(h);
    if (listen(*h, backlog) < 0) {
        err = WSAGetLastError();
    }
    socket_setnonblocking(h);
    return err;
}

/*-------------------------------------------------------------------------*\
* Accept with timeout
\*-------------------------------------------------------------------------*/
int socket_accept(socket_t* h, socket_t* pa, sockaddr* addr, socklen_t* len, timeout_t tm)
{
    sockaddr daddr;
    socklen_t dlen = sizeof(daddr);

    if (*h == SOCKET_INVALID) {
        return IO_CLOSED;
    }

    if (addr == NULL) {
        addr = &daddr;
    }

    if (len == NULL) {
        len = &dlen;
    }

    for ( ;; ) {
        int err;
        /* try to get client socket */
        if ((*pa = accept(*h, addr, len)) != SOCKET_INVALID) {
            return IO_DONE;
        }

        /* find out why we failed */
        err = WSAGetLastError(); 

        /* if we failed because there was no connectoin, keep trying */
        if (err != WSAEWOULDBLOCK && err != WSAECONNABORTED) {
            return err;
        }

        /* call select to avoid busy wait */
        if ((err = socket_waitfd(h, WAITFD_R, tm)) != IO_DONE) {
            return err;
        }
    } 

    /* can't reach here */
    return IO_UNKNOWN; 
}

/*-------------------------------------------------------------------------*\
* Send with timeout
* On windows, if you try to send 10MB, the OS will buffer EVERYTHING 
* this can take an awful lot of time and we will end up blocked. 
* Therefore, whoever calls this function should not pass a huge buffer.
\*-------------------------------------------------------------------------*/
int socket_send(socket_t* h, const char *data, size_t count, size_t *sent, timeout_t tm)
{
    int err;
    *sent = 0;

    if (*h == SOCKET_INVALID) {
        return IO_CLOSED;
    }

    /* loop until we send something or we give up on error */
    for ( ;; ) {
        /* try to send something */
		int put = send(*h, data, (int)count, 0);

        /* if we sent something, we are done */
        if (put > 0) {
            *sent = put;
            return IO_DONE;
        }


        /* deal with failure */
        err = WSAGetLastError(); 
        /* we can only proceed if there was no serious error */
        if (err != WSAEWOULDBLOCK) {
            return err;
        }

        /* avoid busy wait */
        if ((err = socket_waitfd(h, WAITFD_W, tm)) != IO_DONE) {
            return err;
        }
    } 

    return IO_UNKNOWN;
}

int socket_recv(socket_t* h, char *data, size_t count, size_t *got, timeout_t tm)
{
    int err;
    *got = 0;

    if (*h == SOCKET_INVALID) {
        return IO_CLOSED;
    }

    for ( ;; ) {
        int taken = recv(*h, data, (int) count, 0);
        if (taken > 0) {
            *got = taken;
            return IO_DONE;
        }

        if (taken == 0) {
            return IO_CLOSED;
        }

        err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            return err;
        }

        if ((err = socket_waitfd(h, WAITFD_R, tm)) != IO_DONE) {
            return err;
        }
    }
    return IO_UNKNOWN;
}


void socket_setblocking(socket_t* h)
{
    u_long argp = 0;
    ioctlsocket(*h, FIONBIO, &argp);
}

void socket_setnonblocking(socket_t* h)
{
    u_long argp = 1;
    ioctlsocket(*h, FIONBIO, &argp);
}

int socket_gethostbyaddr(const char *addr, socklen_t len, struct hostent **hp)
{
    *hp = gethostbyaddr(addr, len, AF_INET);
    if (*hp) {
        return IO_DONE;
    } else {
        return WSAGetLastError();
    }
}

int socket_gethostbyname(const char *hostname, struct hostent **hp)
{
    *hp = gethostbyname(hostname);
    if (*hp) {
        return IO_DONE;
    } else {
        return  WSAGetLastError();
    }
}

const char *socket_strerror(int err)
{
    switch (err) {
        case IO_DONE: return NULL;
        case IO_CLOSED: return "closed";
        case IO_TIMEOUT: return "timeout";

        case WSAEINTR: return "Interrupted function call";
        case WSAEACCES: return "Permission denied";
        case WSAEFAULT: return "Bad address";
        case WSAEINVAL: return "Invalid argument";
        case WSAEMFILE: return "Too many open files";
        case WSAEWOULDBLOCK: return "Resource temporarily unavailable";
        case WSAEINPROGRESS: return "Operation now in progress";
        case WSAEALREADY: return "Operation already in progress";
        case WSAENOTSOCK: return "Socket operation on nonsocket";
        case WSAEDESTADDRREQ: return "Destination address required";
        case WSAEMSGSIZE: return "Message too long";
        case WSAEPROTOTYPE: return "Protocol wrong type for socket";
        case WSAENOPROTOOPT: return "Bad protocol option";
        case WSAEPROTONOSUPPORT: return "Protocol not supported";
        case WSAESOCKTNOSUPPORT: return "Socket type not supported";
        case WSAEOPNOTSUPP: return "Operation not supported";
        case WSAEPFNOSUPPORT: return "Protocol family not supported";
        case WSAEAFNOSUPPORT: 
            return "Address family not supported by protocol family"; 
        case WSAEADDRINUSE: return "Address already in use";
        case WSAEADDRNOTAVAIL: return "Cannot assign requested address";
        case WSAENETDOWN: return "Network is down";
        case WSAENETUNREACH: return "Network is unreachable";
        case WSAENETRESET: return "Network dropped connection on reset";
        case WSAECONNABORTED: return "Software caused connection abort";
        case WSAECONNRESET: return "Connection reset by peer";
        case WSAENOBUFS: return "No buffer space available";
        case WSAEISCONN: return "Socket is already connected";
        case WSAENOTCONN: return "Socket is not connected";
        case WSAESHUTDOWN: return "Cannot send after socket shutdown";
        case WSAETIMEDOUT: return "Connection timed out";
        case WSAECONNREFUSED: return "Connection refused";
        case WSAEHOSTDOWN: return "Host is down";
        case WSAEHOSTUNREACH: return "No route to host";
        case WSAEPROCLIM: return "Too many processes";
        case WSASYSNOTREADY: return "Network subsystem is unavailable";
        case WSAVERNOTSUPPORTED: return "Winsock.dll version out of range";
        case WSANOTINITIALISED: 
            return "Successful WSAStartup not yet performed";
        case WSAEDISCON: return "Graceful shutdown in progress";
        case WSAHOST_NOT_FOUND: return "Host not found";
        case WSATRY_AGAIN: return "Nonauthoritative host not found";
        case WSANO_RECOVERY: return "Nonrecoverable name lookup error"; 
        case WSANO_DATA: return "Valid name, no data record of requested type";
        default: return "Unknown error";
    }
}

