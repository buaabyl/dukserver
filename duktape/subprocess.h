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

#ifndef SUBPROCESS_H_F470BFDE_579D_11E5_A8CF_005056C00008_INCLUDED_
#define SUBPROCESS_H_F470BFDE_579D_11E5_A8CF_005056C00008_INCLUDED_
#include <fcntl.h>

//example:
//  write(stdin_fileno, buffer, size)
//  read(stdout_fileno, buffer, size)
typedef struct {
    void* proc;
    int pid;
    int stdin_fileno;
    int stdout_fileno;
}subprocess_t;

//windows:
//  file != NULL: file + " " + args[1] + " " + args[2] + " " + ...
//  file == NULL: args[0] + " " + args[1] + " " + args[2] + " " + ...
//
//linux: 
//  execvp(file, args);
//
subprocess_t* psopen(char* file, char* args[]);

//wait child exit or terminated, then close all handle.
int pswait(subprocess_t* ps);

//kill child if not exist nor terminated, then close all handle, and free all.
void psclose(subprocess_t* ps);

#endif

