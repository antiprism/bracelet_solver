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

#include "pattern.h"
#include "utils.h"
#include <limits.h>
#include <stdio.h>

using std::vector;
using std::string;
using std::map;

vector<int> get_next_strings(const vector<int> &strings_in,
                             const vector<int> &knots, int offset,
                             bool prefer_pure = true, int *line_score = 0);
int increment(vector<int> &digits);

Pattern::Pattern()
    : soln_no(0), soln_strs_no(0), soln_perm_no(0), soln_var_no(0),
      score(INT_MAX), score_limit(INT_MAX), prefer_pure(true), descent_cnt(0)
{
}

void Pattern::print_strings_and_knots(const KnotColors &k, FILE *file) const
{
  vector<int> str = strings;
  for (unsigned int i = 0; i <= k.knots.size(); i++) {
    int offset = i % 2;
    fprintf(file, "      ");
    for (unsigned int j = 0; j < str.size(); j++)
      fprintf(file, "%c ", k.get_colour(str[j]));
    fprintf(file, "\n");
    if (i < k.knots.size()) {
      fprintf(file, "%3d:  ", i + 1);
      fprintf(file, " %s", offset ? "  " : "");
      for (unsigned int j = 0; j < k.knots[i].size(); j++)
        fprintf(file, "%c   ", k.get_colour(k.knots[i][j]));
      fprintf(file, "\n");
      const vector<int> &knts = knots[i];
      vector<int> str_tmp = get_next_strings(str, knts, offset);
      str = str_tmp;
    }
    else
      str = strings;
  }
}

void Pattern::print_generator_pattern(const KnotColors &k, FILE *file) const
{
  vector<int> str = strings;
  for (unsigned int i = 0; i < str.size(); i++)
    fprintf(file, "%c ", k.get_colour(str[i]));
  fprintf(file, "\n\n");
  for (unsigned int i = 0; i < k.knots.size(); i++) {
    int offset = i % 2;
    const vector<int> &knts = knots[i];
    vector<int> str_next = get_next_strings(str, knts, offset);
    // for(unsigned int s=0; s<str.size(); s++)
    //   fprintf(stdout, "%d ", str[s]);
    // fprintf(stdout, "\n");
    // for(unsigned int s=0; s<str_next.size(); s++)
    //  fprintf(stdout, "%d ", str_next[s]);
    // fprintf(stdout, "\n");
    for (unsigned int j = 0; j < k.knots[i].size(); j++) {
      int K = k.knots[i][j];
      int TL = str[2 * j + offset];
      int TR = str[2 * j + offset + 1];
      // int BL = str_next[2*j+offset];
      int BR = str_next[2 * j + offset + 1];
      // If optional switch or not to match the solution type, otherwise
      // switch as necessary
      bool switch_strs = (TL == TR) ? (prefer_pure) : (TL == BR);
      bool knot_left_str = (TL == K);
      char knot_types[] = "RrdD";
      int idx = 2 * switch_strs + knot_left_str;
      // fprintf(stdout, "%sswitch, knot %s -> type[%d]='%c'\n",
      //      switch_strs?"": "don't ", knot_left_str?"left":"right",
      //      idx, knot_types[idx]);
      // fprintf(stdout,"[%d,%d](%d,%d) = (%d,%d [%d] %d,%d)\n", i, j,
      // 2*j+offset, 2*j+offset+1, TL, TR, K, BL, BR);
      fprintf(file, "%c", knot_types[idx]);
    }
    str = str_next;
    fprintf(file, "\n");
  }
}

void Pattern::print_pattern(const KnotColors &k, int sol_no, FILE *file) const
{
  fprintf(file, "\n-------------------------------------------\n"
                "SOLUTION %5d (score: %4d)  (ref: %d/%d/%d)\n",
          sol_no, score, soln_strs_no, soln_perm_no, soln_var_no);

  fprintf(file, "Text pattern format:\n\n");
  print_strings_and_knots(k, file);
  fprintf(file, "\n");
  fprintf(file, "Generator pattern format:\n\n");
  print_generator_pattern(k, file);
  fprintf(file, "\n\n");
}

int Pattern::check_line(int line, const vector<int> &strings_in,
                        const KnotColors &k, vector<Pattern> &sols,
                        bool try_optimal, int cur_score)
{
  // Knots either switch the input strings or leave them as they are.
  // Permute through all knot possibilities for this line. If output
  // strings are able to make the line below, recurse.
  descent_cnt++;
  if (descent_cnt % 100000 == 0) {
    fprintf(stderr, ".");
    if (descent_cnt % 1000000 == 0)
      fprintf(stderr, " ");
    if (descent_cnt % 7000000 == 0)
      fprintf(stderr, "\n");
    fflush(stderr);
  }

  // knots is effectively a binary numbery
  if (try_optimal && prefer_pure)                    // try pure knots only
    fill(knots[line].begin(), knots[line].end(), 1); // 11...11
  else // start for trying all  or try mixed knots only
    fill(knots[line].begin(), knots[line].end(), 0); // 00...00

  const int first = line % 2; // first string that will be knotted
  bool valid;
  while (true) {
    // Pairs of input strings may be switched or left by the knots
    int line_score;
    vector<int> strings_out = get_next_strings(strings_in, knots[line], first,
                                               prefer_pure, &line_score);

    score = cur_score + line_score;
    // fprintf(stdout, "score=%d, line_score=%d\n", score, line_score);
    valid = (line_score >= 0) && (score <= score_limit);

    // check each knot in the next line (wrap) is met by at least one
    // string of the same colour
    const int next_line = (line < (int)knots.size() - 1) ? line + 1 : 0;
    int next_first = !first;
    for (unsigned int i = 0; i < k.knots[next_line].size(); i++) {
      if (k.knots[next_line][i] != strings_out[next_first + 2 * i] &&
          k.knots[next_line][i] != strings_out[next_first + 2 * i + 1]) {
        valid = false;
        break;
      }
    }

    // Check whether strings match when wrapping to beginning
    if (valid && !next_line)
      valid = (strings_out == strings);

    if (valid) {
      if (next_line)
        valid = check_line(line + 1, strings_out, k, sols, try_optimal, score);
    }

    if (valid && !next_line) {
      // print_pattern(k, stderr);
      if (sols.size() == 0 || sols[0].score >= score) {
        if (sols.size() && sols[0].score > score) {
          sols.clear();
          score_limit = score;
        }
        sols.push_back(*this);
        // fprintf(stderr, "\nfound one (score=%d, line=%d)! (sols.size=%d\n",
        //    score, line, (int)sols.size());
      }
      increment_var();
    }

    if (try_optimal) // only need to test each line once
      break;

    int l_cnt = (knots[line].size() + 1) / 2;
    vector<int> half;
    vector<int> &knots_line = only_symmetric ? half : knots[line];
    if (only_symmetric) {
      half.insert(half.end(), knots[line].begin(), knots[line].begin() + l_cnt);
    }

    if (!increment(knots_line))
      break;

    if (only_symmetric) {
      int r_cnt = knots[line].size() - l_cnt;
      knots[line] = half;
      knots[line].insert(knots[line].end(), half.rbegin(),
                         half.rbegin() + r_cnt);
    }
  }

  return valid; // none of the knot permutations for this line were valid
}

vector<Pattern> Pattern::solve(const KnotColors &k, bool optimal)
{
  // initialise knots
  knots.clear();
  knots.resize(k.knots.size());
  for (unsigned int i = 0; i < k.knots.size(); i++)
    knots[i].resize(k.knots[i].size());
  vector<Pattern> sols;
  check_line(0, strings, k, sols, optimal);
  return sols;
}

string Pattern::get_strings_text(map<int, char> &idx2col)
{
  string txt;
  const int sz = strings.size();
  for (int i = 0; i < sz; i++) {
    txt += idx2col[strings[i]];
    if (i < sz - 1)
      txt += " ";
  }

  return txt;
}

vector<int> get_next_strings(const vector<int> &strings_in,
                             const vector<int> &knots, int offset,
                             bool prefer_pure, int *line_score)
{
  if (line_score)
    *line_score = 0;
  vector<int> strings_out = strings_in;
  for (unsigned int i = 0; i < knots.size(); i++) {
    if (line_score)
      *line_score += knots[i] != prefer_pure;
    const int idx = offset + 2 * i;
    if (knots[i]) {
      // only count solutions that don't swap strings of the same colour
      if (line_score && !prefer_pure &&
          strings_out[idx] == strings_out[idx + 1]) {
        *line_score = -1; // redundant solution
        break;
      }
      std::swap(strings_out[idx], strings_out[idx + 1]);
    }
    else {
      if (line_score && prefer_pure == 1 &&
          strings_out[idx] == strings_out[idx + 1]) {
        *line_score = -1; // redundant solution
        break;
      }
    }
  }

  return strings_out;
}

int increment(vector<int> &digits)
{
  int sz = digits.size();
  digits[sz - 1] += 1;
  for (int i = 0; i < sz; i++) {
    const int idx = (sz - i) - 1;
    if (digits[idx] > 1) {
      if (idx == 0) // reached maximum
        return false;
      else {
        digits[idx - 1] += 1;       // increment previous digit
        if (digits[idx - 1] <= 1) { // valid, so update following digits
          for (int j = idx; j < sz - 1; j++)
            digits[j] = 0;
          digits.back() = 0;
          break;
        }
      }
    }
  }
  return true;
}
