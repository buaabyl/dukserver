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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "subprocess.h"

//  parent ---write---> pipe ---read---> child
//         inheritance        heritance
static int _create_pipe_stdin(int* stdin_rd, int* stdin_wr);

//  parent <---read--- pipe <---write--- child
//         inheritance       heritance
static int _create_pipe_stdout(int* stdout_rd, int* stdout_wr);


int _create_pipe_stdin(int* stdin_rd, int* stdin_wr)
{
    int h[2];
    int h_rd;

    if (pipe(h) < 0) {
        return -1;
    }

    h_rd = dup(h[0]);
    close(h[0]);

    *stdin_rd = h_rd;
    *stdin_wr = h[1];

    return 0;
}

int _create_pipe_stdout(int* stdout_rd, int* stdout_wr)
{
    int h[2];
    int h_wr;

    if (pipe(h) < 0) {
        return -1;
    }

    h_wr = dup(h[1]);
    close(h[1]);

    *stdout_rd = h[0];
    *stdout_wr = h_wr;

    return 0;
}

subprocess_t* psopen(char* file, char* args[])
{
    subprocess_t* ps;
    pid_t pid; 
    int stdin_wr;
    int stdin_rd;
    int stdout_wr;
    int stdout_rd;

    if (_create_pipe_stdin(&stdin_rd, &stdin_wr) < 0) {
        return NULL;
    }
    if (_create_pipe_stdout(&stdout_rd, &stdout_wr) < 0) {
        close(stdin_rd);
        close(stdin_wr);
        return NULL;
    }

    pid = vfork();
    if (pid < 0) {
        close(stdin_rd);
        close(stdin_wr);
        close(stdout_rd);
        close(stdout_wr);
        return NULL;
    }

    if (pid == 0) {
        //_exit() do not flush then close standard I/O
        //so I just use exit()
        if (dup2(stdin_rd, STDIN_FILENO) < 0) {
            exit(-1);
        }
        if (dup2(stdout_wr, STDOUT_FILENO) < 0) {
            exit(-1);
        }
        execvp(args[0], args);
        exit(-1);
    }
    
    close(stdin_rd);
    close(stdout_wr);

    ps = (subprocess_t*)malloc(sizeof(subprocess_t));
    ps->pid             = pid;
    ps->proc            = (void*)pid;
    ps->stdin_fileno    = stdin_wr;
    ps->stdout_fileno   = stdout_rd;

    return ps;
}


int pswait(subprocess_t* ps)
{
    int status;
    int returncode = 0;

    if (ps->pid > 0) {
        if (waitpid(ps->pid , &status, 0) == -1) {
            returncode = -1;
        } else {
            returncode = (char)(status >> 8);
        }
        ps->pid = 0;

        close(ps->stdin_fileno);
        close(ps->stdout_fileno);
    }

    return returncode;
}

void psclose(subprocess_t* ps)
{
    int i;
    int signals[] = {SIGINT, SIGTERM, SIGABRT, SIGKILL};

    if (ps->pid > 0) {
        close(ps->stdin_fileno);
        close(ps->stdout_fileno);

        for (i = 0;i < sizeof(signals)/sizeof(signals[0]);i++) {
            if (kill(ps->pid, 0) == -1) {
                break;
            }
            kill(ps->pid, signals[i]);
            usleep(10 * 1000);
        }

        ps->pid = 0;
    }

    free(ps);
}


