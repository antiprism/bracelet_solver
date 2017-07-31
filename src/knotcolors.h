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

#ifndef KNOTCOLORS_H
#define KNOTCOLORS_H

#include "status.h"
#include <map>
#include <string>
#include <vector>

class Pattern;

class KnotColors {
public:
  std::vector<std::vector<int>> knots;
  std::vector<int> perm_sections;

  std::vector<int> req_colour_cnts;
  std::map<char, int> col2idx;
  std::map<int, char> idx2col;

  std::vector<int> fixed_strings;
  std::vector<int> excess_strings;
  std::vector<int> right;

  int num_strings;
  int num_colours;
  int num_free_strings;
  bool is_symmetric;

  Status read_knot_lines(FILE *ifile, std::vector<std::string> &knot_lines);
  void process_knots(const std::vector<std::string> &knot_lines);
  char get_colour(int idx) const;
  int get_idx(char col) const;
  int num_knots() const;

  Status read(FILE *ifile);

  int check_perm(int num_lines);
  void get_perm_sections();
  void update_symmetry();
  void print_knots(FILE *file) const;

  std::vector<Pattern> solve(bool prefer_pure, bool optimal);
};

void print_cnts(FILE *file, const std::vector<int> &cnts, const KnotColors &k,
                const std::string &desc);
bool symmetric(const std::vector<int> &vec);
#endif // KNOTCOLORS_H
