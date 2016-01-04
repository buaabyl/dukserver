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

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h> 

#include <windows.h>
#include <io.h>

#include "subprocess.h"

//  parent ---write---> pipe ---read---> child
//         inheritance        heritance
static BOOL _create_pipe_stdin(HANDLE* stdin_rd, HANDLE* stdin_wr);

//  parent <---read--- pipe <---write--- child
//         inheritance       heritance
static BOOL _create_pipe_stdout(HANDLE* stdout_rd, HANDLE* stdout_wr);

static int _parse_environment_path_var(char** * result);
static void _destroy_environment_path_result(char** result, int nr_paths);
static BOOL _is_file_exists(const char* fn);
static char* _find_image_path(char* file);
static char* _combine_args(char* args[]);


BOOL _create_pipe_stdin(HANDLE* stdin_rd, HANDLE* stdin_wr)
{
    HANDLE pipe_rd;
    HANDLE pipe_wr;
    HANDLE pipe_wr_dup;
    SECURITY_ATTRIBUTES attr; 
    BOOL success;

    //prepare stdout pipe
    ZeroMemory(&attr, sizeof(attr));
    attr.nLength               = sizeof(attr);
    attr.bInheritHandle        = TRUE;
    attr.lpSecurityDescriptor  = NULL;
    success = CreatePipe(&pipe_rd, &pipe_wr, &attr, 0);
    if (!success) {
        return FALSE;
    }

    success = DuplicateHandle(GetCurrentProcess(), pipe_wr,
            GetCurrentProcess(), &pipe_wr_dup,
            0, FALSE, DUPLICATE_SAME_ACCESS);
    if (!success) {
        CloseHandle(pipe_rd);
        CloseHandle(pipe_wr);
        return FALSE;
    }

    CloseHandle(pipe_wr);
    *stdin_rd = pipe_rd;
    *stdin_wr = pipe_wr_dup;

    return TRUE;
}

BOOL _create_pipe_stdout(HANDLE* stdout_rd, HANDLE* stdout_wr)
{
    HANDLE pipe_wr;
    HANDLE pipe_rd;
    HANDLE pipe_rd_dup;
    SECURITY_ATTRIBUTES attr; 
    BOOL success;

    //prepare stdout pipe
    ZeroMemory(&attr, sizeof(attr));
    attr.nLength               = sizeof(attr);
    attr.bInheritHandle        = TRUE;
    attr.lpSecurityDescriptor  = NULL;
    success = CreatePipe(&pipe_rd, &pipe_wr, &attr, 0);
    if (!success) {
        return FALSE;
    }

    success = DuplicateHandle(GetCurrentProcess(), pipe_rd,
            GetCurrentProcess(), &pipe_rd_dup,
            0, FALSE, DUPLICATE_SAME_ACCESS);
    if (!success) {
        CloseHandle(pipe_rd);
        CloseHandle(pipe_wr);
        return FALSE;
    }

    CloseHandle(pipe_rd);
    *stdout_rd = pipe_rd_dup;
    *stdout_wr = pipe_wr;

    return TRUE;
}

int _parse_environment_path_var(char** * result)
{
    char* env_path;
    int nr_bytes;//inclued '\0'

    char** paths = NULL;
    int nr_paths = 0;

    char* pend;
    char* prev;
    char* p;
    char* q;

    int i;

    ////Test
    //SetEnvironmentVariable("PATH", "C:\\Windows ; C:\\Windows\\System32; ;");

    nr_bytes = GetEnvironmentVariable("PATH", NULL, 0);
    if (nr_bytes > 0) {
        env_path = (char*)malloc(nr_bytes);
        GetEnvironmentVariable("PATH", env_path, nr_bytes);
        for (i = 0;i < nr_bytes;i++) {
            if (env_path[i] == '/') {
                env_path[i] = '\\';
            }
        }

        prev = NULL;
        pend = env_path + nr_bytes-1;
        p = env_path;

        while (p < pend) {
            if ((prev == NULL) && !isspace(*p)) {
                prev = p;
                continue;
            } 
            
            if (*p == ';') {
                if (prev != NULL) {
                    for (q = p;prev < q;q--) {
                        if (!isspace(q[-1])) {
                            break;
                        }
                    }

                    if (q - prev > 0) {
                        if (paths == NULL) {
                            nr_paths = 0;
                            paths = (char**)malloc(sizeof(char*) * (nr_paths + 1));
                        } else {
                            paths = (char**)realloc(paths, sizeof(char*) * (nr_paths + 1));
                        }

                        paths[nr_paths] = (char*)malloc(q - prev + 1);
                        memset(paths[nr_paths], 0, q - prev + 1);
                        memcpy(paths[nr_paths], prev, q - prev);

                        nr_paths++;
                    }
                }
                prev = NULL;
            }

            p++;
        }
    }

    *result = paths;

    return nr_paths;
}

void _destroy_environment_path_result(char** result, int nr_paths)
{
    int i;

    for (i = 0;i < nr_paths;i++) {
        free(result[i]);
    }

    free(result);
}

BOOL _is_file_exists(const char* fn)
{
    struct _stat32 st;

    if (_stat32(fn, &st) != 0) {
        return FALSE;
    }

    if (st.st_mode & _S_IFDIR) {
        return FALSE;
    }

    return TRUE;
}

char* _find_image_path(char* file)
{
    char* appname = NULL;

    char** paths;
    int nr_paths;
    int path_size;
    char* args0;
    int args0_size;
    char ext[4];
    int i;

    ////////////////////////////////////////////////////////
    //check extension name
    args0_size = strlen(file);
    if (args0_size <= 0) {
        return NULL;
    }

    args0 = (char*)malloc(args0_size + 4 + 1);
    memset(args0, 0, args0_size + 4 + 1);
    memcpy(args0, file, args0_size);
    memcpy(ext, args0 + args0_size - 4, 4);

    ext[0] = tolower(ext[0]);
    ext[1] = tolower(ext[1]);
    ext[2] = tolower(ext[2]);
    ext[3] = tolower(ext[3]);
    if (strncmp(ext, ".exe", 4) != 0) {
        strcat(args0, ".exe");
    }
    args0_size = strlen(args0);

    if ((args0[0] == '.') && (args0[1] == '\\')) {
        return args0;
    } 
    if ((args0[0] == '.') && (args0[1] == '.') && (args0[2] == '\\')) {
        return args0;
    } 
    if (_is_file_exists(args0)) {
        return args0;
    }

    ////////////////////////////////////////////////////////
    //build appname from PATH environment
    nr_paths = _parse_environment_path_var(&paths);
    if (nr_paths <= 0) {
        return args0;
    }

    for (i = 0;i < nr_paths;i++) {
        path_size = strlen(paths[i]);
        appname = (char*)malloc(path_size + 1 + args0_size + 1);
        memset(appname, 0, path_size + 1 + args0_size + 1);
        memcpy(appname, paths[i], path_size);
        if (appname[path_size-1] != '\\') {
            appname[path_size++] = '\\';
        }
        memcpy(appname+path_size, args0, args0_size);

        if (_is_file_exists(appname)) {
            free(args0);
            args0 = appname;
            break;
        }

        free(appname);
        appname = NULL;
    }

    _destroy_environment_path_result(paths, nr_paths);

    return args0;
}

char* _combine_args(char* args[])
{
    char* cmdline = NULL;
    int cmdline_size = 0;
    int n_str;

    while (*args != NULL) {
        n_str = strlen(*args);
        if (cmdline == NULL) {
            cmdline_size = 0;
            cmdline = (char*)malloc(n_str + 1);
        } else {
            cmdline = (char*)realloc(cmdline, cmdline_size + 1 + n_str + 1);
        }

        memcpy(&cmdline[cmdline_size], *args, n_str);
        cmdline_size += n_str;
        if (*(args+1) != NULL) {
            cmdline[cmdline_size++] = ' ';
        }
        cmdline[cmdline_size] = '\0';

        args++;
    }

    return cmdline;
}


subprocess_t* psopen(char* file, char* args[])
{
    subprocess_t* ps = NULL;
    HANDLE stdout_wr = INVALID_HANDLE_VALUE;
    HANDLE stdout_rd = INVALID_HANDLE_VALUE;
    HANDLE stdin_wr  = INVALID_HANDLE_VALUE;
    HANDLE stdin_rd  = INVALID_HANDLE_VALUE;
    int h_stdout_rd  = -1;
    int h_stdin_wr   = -1;
    char* appname = NULL;
    char* cmdline = NULL;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL success;

    ////////////////////////////////////////////////////////
    if ((args == NULL) || (args[0] == NULL)) {
        return NULL;
    }

    if (file != NULL) {
        appname = _find_image_path(file);
    } else {
        appname = _find_image_path(args[0]);
    }
    if (appname == NULL) {
        return NULL;
    }
    cmdline = _combine_args(args);
    if (cmdline == NULL) {
        free(appname);
        return NULL;
    }

    ////////////////////////////////////////////////////////
    success = _create_pipe_stdout(&stdout_rd, &stdout_wr);
    if (!success) {
        goto L_ERROR;
    }

    success = _create_pipe_stdin(&stdin_rd, &stdin_wr);
    if (!success) {
        goto L_ERROR;
    }

    ////////////////////////////////////////////////////////
    h_stdin_wr = _open_osfhandle((intptr_t)stdin_wr, 0);
    if (h_stdin_wr == -1) {
        goto L_ERROR;
    }
    stdin_wr = INVALID_HANDLE_VALUE;

    h_stdout_rd = _open_osfhandle((intptr_t)stdout_rd, 0);
    if (h_stdin_wr == -1) {
        goto L_ERROR;
    }
    stdout_rd = INVALID_HANDLE_VALUE;

    ////////////////////////////////////////////////////////
    ZeroMemory(&si, sizeof(si));
    si.cb           = sizeof(si);
    si.hStdInput    = stdin_rd;
    si.hStdOutput   = stdout_wr;
    si.hStdError    = stdout_wr;
    si.dwFlags      = STARTF_USESTDHANDLES;

    // Start the child process.
    // XXX: found exe from PATH, then set lpApplicationName!
    success = CreateProcess(
        appname,            //lpApplicationName
        cmdline,            //lpCommandLine
        //NULL,               //lpApplicationName
        //cmdline,            //lpCommandLine
        NULL,               //lpProcessAttributes for Security
        NULL,               //lpThreadAttributes for Security
        TRUE,               //bInHeritanceHandles
        CREATE_NO_WINDOW,   //dwCreationFlags
        NULL,               //lpEnvironment
        NULL,               //lpCurrentDirectory
        &si,                //lpStartupInfo
        &pi                 //lpProcessInformation
    );

    if (!success) {
        goto L_ERROR;
    }

    ////////////////////////////////////////////////////////
    free(appname);
    appname = NULL;
    free(cmdline);
    cmdline = NULL;

    CloseHandle(pi.hThread);

    // handle have hold by child, no need to have it.
    CloseHandle(stdout_wr);
    stdout_wr = INVALID_HANDLE_VALUE;
    CloseHandle(stdin_rd);
    stdin_rd = INVALID_HANDLE_VALUE;

    ps = (subprocess_t*)malloc(sizeof(subprocess_t));
    ps->pid             = GetProcessId(pi.hProcess);
    ps->proc            = (void*)pi.hProcess;
    ps->stdin_fileno    = h_stdin_wr;
    ps->stdout_fileno   = h_stdout_rd;

    return ps;

L_ERROR:
    if (appname != NULL) {
        free(appname);
    }
    if (cmdline != NULL) {
        free(cmdline);
    }
    if (h_stdout_rd != -1) {
        close(h_stdout_rd);
    }
    if (h_stdin_wr != -1) {
        close(h_stdin_wr);
    }
    if (stdout_wr != INVALID_HANDLE_VALUE) {
        CloseHandle(stdout_wr);
    }
    if (stdout_rd != INVALID_HANDLE_VALUE) {
        CloseHandle(stdout_rd);
    }
    if (stdin_wr != INVALID_HANDLE_VALUE) {
        CloseHandle(stdin_wr);
    }
    if (stdin_rd != INVALID_HANDLE_VALUE) {
        CloseHandle(stdin_rd);
    }

    return NULL;
}

int pswait(subprocess_t* ps)
{
    DWORD exitcode = 0;

    if (ps == NULL) {
        return -1;
    }

    if ((HANDLE)ps->proc != INVALID_HANDLE_VALUE) {
        WaitForSingleObject((HANDLE)ps->proc, INFINITE);
        if (!GetExitCodeProcess((HANDLE)ps->proc, &exitcode)) {
            exitcode = -1;
        }

        close(ps->stdin_fileno);
        close(ps->stdout_fileno);
        CloseHandle((HANDLE)ps->proc);

        ps->proc = (void*)INVALID_HANDLE_VALUE;
    }

    return exitcode;
}

void psclose(subprocess_t* ps)
{
    if (ps == NULL) {
        return;
    }

    if ((HANDLE)ps->proc != INVALID_HANDLE_VALUE) {
        close(ps->stdin_fileno);
        close(ps->stdout_fileno);

        WaitForSingleObject((HANDLE)ps->proc, 10);
        CloseHandle((HANDLE)ps->proc);

        ps->proc = (void*)INVALID_HANDLE_VALUE;
    }

    free(ps);
}



