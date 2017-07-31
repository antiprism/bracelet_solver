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

#ifndef PATTERN_H
#define PATTERN_H

#include "knotcolors.h"
#include <vector>

class Pattern {
public:
  int soln_no;      // for all solutions
  int soln_strs_no; // for a set of free colours
  int soln_perm_no; // for a permutation of free colours
  int soln_var_no;  // for knotting variation of a permutation
  int score;
  int score_limit; // stop processing if score is greater than this

  bool prefer_pure;    // find solution with most pure/mixed knots
  bool only_symmetric; // only look for symmetric solutions

  std::vector<int> strings;            // start strings
  std::vector<std::vector<int>> knots; // knots (swap strings or not)

  long descent_cnt; // number of recursion descents

  Pattern();
  void set_score_limit(int lim) { score_limit = lim; }
  void set_strings(const std::vector<int> &strs) { strings = strs; }
  void set_prefer_pure(int pure) { prefer_pure = pure; }
  void increment_strs()
  {
    soln_strs_no++;
    soln_perm_no = 0;
    soln_var_no = 0;
  }
  void increment_perm()
  {
    soln_perm_no++;
    soln_var_no = 0;
  }
  void increment_var()
  {
    soln_var_no++;
    soln_no++;
  }

  int check_line(int line, const std::vector<int> &strs, const KnotColors &k,
                 std::vector<Pattern> &sols, bool try_optimal = false,
                 int cur_score = 0);
  std::vector<Pattern> solve(const KnotColors &k, bool optimal = false);
  std::string get_strings_text(std::map<int, char> &idx2col);

  void print_pattern(const KnotColors &k, int sol_no, FILE *file) const;
  void print_strings_and_knots(const KnotColors &k, FILE *file) const;
  void print_generator_pattern(const KnotColors &k, FILE *file) const;
};

#endif // PATTERN_H
