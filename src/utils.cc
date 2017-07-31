/*
   Copyright (c) 2014-2017 Adrian Rossiter <adrian@antiprism.com>
                           http://www.antiprism.com/other/bracelet_solver/

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

      The above copyright notice and this permission notice shall be included
      in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

/* \file utils.cc
   \brief utility routines for maths operations, text operations,
   I/O conversions, etc
*/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <ctype.h>
#include <limits.h>
#include <map>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "utils.h"

using std::string;
using std::vector;
using std::map;

const char WHITESPACE[] = " \t\r\n\f\v";

Status read_int(const char *str, int *i)
{
  char buff;
  if (sscanf(str, " %d %c", i, &buff) != 1)
    return Status::error("not an integer");

  if (*i == INT_MAX)
    return Status::error("integer too large\n");

  return Status::ok();
}

int read_line(FILE *file, char **line)
{

  int linesize = 128;
  *line = (char *)malloc(linesize);
  if (!*line)
    return -1;

  int offset = 0;
  while (true) {
    if (!fgets(*line + offset, linesize - offset, file)) {
      if (offset != 0)
        return 0;
      else {
        *(*line + offset) = '\0'; // terminate the line
        return (ferror(file)) ? -1 : 1;
      }
    }
    int len = offset + strlen(*line + offset);
    if ((*line)[len - 1] == '\n') {
      (*line)[len - 1] = 0;
      return 0;
    }
    offset = len;

    char *newline = (char *)realloc(*line, linesize * 2);
    if (!newline)
      return -1;
    *line = newline;
    linesize *= 2;
  }
}

int split_line(char *line, vector<char *> &parts, const char *delims,
               bool strict)
{
  parts.clear();
  if (!delims)
    delims = WHITESPACE;

  if (strict) {
    char *cur = line;
    parts.push_back(cur);                    // always an entry, even if null
    while (*(cur += strcspn(cur, delims))) { // quit at end of string
      *cur = '\0';                           // terminate part
      cur++;                                 // start of next part
      parts.push_back(cur);                  // add even if final null
    }
    /*while(*cur) {                             // quit at end of string
       cur += strcspn(cur, delims);           // next delimiter
       if(*cur) {                             // part ended with delimiter
          *cur = '\0';                        // terminate part
          cur++;                              // start of next part
          parts.push_back(cur);                // add even if final null
       }
    }*/
  }
  else {
    char *val;
    if (!(val = strtok(line, delims)))
      return 0;

    parts.push_back(val);
    while ((val = strtok(nullptr, delims)))
      parts.push_back(val);
  }

  return parts.size();
}

void backslash_to_forward(string &path)
{
  for (char &si : path)
    if (si == '\\')
      si = '/';
}

FILE *fopen_file(string &fpath)
{
  backslash_to_forward(fpath);
  FILE *file = fopen(fpath.c_str(), "r");
  if (file) {
    struct stat st;
    fstat(fileno(file), &st);
    if (S_ISDIR(st.st_mode)) {
      fclose(file);
      file = nullptr;
    }
  }
  return file;
}

string msg_str(const char *fmt, ...)
{
  char message[MSG_SZ];
  va_list args;
  va_start(args, fmt);
  vsnprintf(message, MSG_SZ - 1, fmt, args);
  return message;
}
