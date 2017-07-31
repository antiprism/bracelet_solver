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

/*!\file utils.h
   \brief utility routines for text operations, I/O conversions, etc
*/

#ifndef UTILS_H
#define UTILS_H

#include "status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

const size_t MSG_SZ = 256;

/// Read an integer from a string.
/** The string should only hold the integer, but may
 *  have leading and trailing whitespace.
 * \param str the string holding the integer.
 * \param i used to return the integer.
 * \return status, evaluates to \c true if a valid integer
 *  was read, otherwise \c false.*/
Status read_int(const char *str, int *i);

/// Read a line of arbitrary length
/** The caller is responsible for freeing the memory allocated to line
 *  after each read.
 * \param file the file stream to read from.
 * \param line where the line is returned
 * \return <ul>
 *    <li>\c 0 if the line was read correctly.
 *    <li>\c -1 if memory for \a line could not be allocated.
 *    <li>\c 1 if an final unterminated empty line was read.
 * </ul> */
int read_line(FILE *file, char **line);

/// Split a line into delimited parts
/**\param line the line to split (this will be modified).
 * \param parts the parts of the split line.
 * \param delims the characters to use as delimiters, if \c 0 then use
 *  whitespace characters.
 * \param strict if true then treat every delimiter as a separator, returning
 *  null strings between adjacent delimiters, always returning at least
 *  one part.
 * \return The number of parts. */
int split_line(char *line, std::vector<char *> &parts,
               const char *delims = nullptr, bool strict = false);

/// Open a file
/**\param fpath the path to the file.
 * \return A pointer to the opened file stream. */
FILE *fopen_file(std::string &fpath);

/// Convert a C formated message string to a C++ string
/** Converts the first MSG_SZ-1 characters of the C format string
 * \param fmt the formatted string
 * \param ... the values for the format
 * \return The converted string. */
std::string msg_str(const char *fmt, ...);

/// Convert an integer to a string
/**\param buf a buffer to return the string.
 * \param i the integer.
 * \return The string. */
char *itostr(char *buf, int i);

// inline function definitions

inline std::string itostr(int i)
{
  char buf[MSG_SZ];
  return std::string(itostr(buf, i));
}

inline char *itostr(char *buf, int i)
{
  buf[MSG_SZ - 1] = 0;
  sprintf(buf, "%d", i);
  return buf;
}

#endif // UTILS_H
