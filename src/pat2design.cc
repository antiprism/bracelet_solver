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

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

const char *prog_name = "pat2design";
int read_pattern(FILE *ifile, string &strings, vector<string> &knot_lines,
                 char *msg)
{
  *msg = '\0';
  int file_line_no = 0; // line number in the file

  strings = "";
  char *line = 0;
  while (read_line(ifile, &line) == 0) {
    file_line_no++;
    // fprintf(stderr, "line %d: '%s'\n", file_line_no, line);
    string data;
    for (char *p = line; *p; p++)
      if (!isspace(*p))
        data += *p;
    free(line);

    if (data == "")
      continue;

    if (strings == "") {
      strings = data;
      continue;
    }

    size_t c_idx;
    if ((c_idx = strspn(data.c_str(), "DdRr")) != data.size()) {
      snprintf(msg, MSG_SZ,
               "line %d: knot line included invalid character '%c'",
               file_line_no, data[c_idx]);
      break;
    }

    unsigned int valid_len = strings.size() / 2;
    if (strings.size() % 2 == 0)
      valid_len -= knot_lines.size() % 2;
    if (data.size() != valid_len) {
      snprintf(msg, MSG_SZ, "line %d: knot line had %d knots, should have %d",
               file_line_no, (int)data.size(), (int)valid_len);
      break;
    }

    knot_lines.push_back(data);
  }

  if (!*msg) {
    if (!strings.size())
      sprintf(msg, "file contains no data");
    else if (!knot_lines.size())
      sprintf(msg, "file contains no knot lines");
    else if (!knot_lines.size() % 2)
      sprintf(msg, "file does not contains an even number of knot lines (did "
                   "you include the strings as the first line?)");
  }

  return *msg == 0;
}

string get_next_strings(const string &strings_in, const string &knots,
                        int offset)
{
  string strings_out = strings_in;
  for (unsigned int i = 0; i < knots.size(); i++) {
    if (knots[i] == 'D' || knots[i] == 'd') {
      // fprintf(stderr, "swapping %d and %d\n", offset+2*i,offset+2*i+1);
      std::swap(strings_out[offset + 2 * i], strings_out[offset + 2 * i + 1]);
    }
  }

  return strings_out;
}

string get_knots(const string &strings_in, const string &knots, int offset)
{
  string knot_cols(knots.size(), ' ');
  for (unsigned int i = 0; i < knots.size(); i++)
    knot_cols[i] =
        strings_in[offset + 2 * i + (knots[i] == 'R' || knots[i] == 'd')];

  return knot_cols;
}

void print_strings(const string &strings)
{
  for (unsigned int i = 0; i < strings.size(); i++)
    fprintf(stdout, "%c ", strings[i]);
  fprintf(stdout, "\n");
}

void print_knots(const string &knot_cols, int offset)
{
  fprintf(stdout, "%s", offset ? " " : "");
  for (unsigned int i = 0; i < knot_cols.size(); i++)
    fprintf(stdout, "%c ", knot_cols[i]);
  fprintf(stdout, "\n");
}

void print_result(const string &strings, const vector<string> &knot_lines,
                  bool include_strs = false)
{
  if (include_strs)
    print_strings(strings);

  string next_strings = strings;
  for (unsigned int i = 0; i < knot_lines.size(); i++) {
    int offset = i % 2;
    string knots = get_knots(next_strings, knot_lines[i], offset);
    print_knots(knots, offset);
    next_strings = get_next_strings(next_strings, knot_lines[i], offset);
    if (include_strs)
      print_strings(next_strings);
  }
}

void print_bare_result(const string &strings, const vector<string> &knot_lines)
{
  string next_strings = strings;
  for (unsigned int i = 0; i < knot_lines.size(); i++) {
    int offset = i % 2;
    string knots = get_knots(next_strings, knot_lines[i], offset);
    fprintf(stdout, "%s\n", knots.c_str());
    next_strings = get_next_strings(next_strings, knot_lines[i], offset);
  }
}

int main(int argc, char **argv)
{
  FILE *file;
  if (argc < 2)
    file = stdin;
  else if (argc == 2) {
    string fname = argv[1];
    if (!(file = fopen_file(fname))) {
      fprintf(stderr, "%s: '%s' could not open input file\n", prog_name,
              fname.c_str());
      exit(1);
    }
  }
  else { // (argc>2)
    fprintf(stderr, "%s: too many arguments, give one input file name\n",
            prog_name);
    exit(1);
  }

  string strings;
  vector<string> knot_lines;
  char msg[MSG_SZ];
  if (!read_pattern(file, strings, knot_lines, msg)) {
    fprintf(stderr, "%s: %s\n", prog_name, msg);
    exit(1);
  }

  print_result(strings, knot_lines);
  // fprintf(stdout, "\n\n");
  // print_bare_result(strings, knot_lines);

  return 0;
}
