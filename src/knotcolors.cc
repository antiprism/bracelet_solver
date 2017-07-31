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

#include "knotcolors.h"
#include "pattern.h"
#include "utils.h"
#include <algorithm>
#include <malloc.h>

using std::map;
using std::string;
using std::vector;

void KnotColors::get_perm_sections()
{
  int num_line_prs = knots.size() / 2; // section unit is two lines
  for (int i = 1; i < num_line_prs; i++) {
    // check if section divides length and permutes colours
    if ((num_line_prs % i) == 0 && check_perm(2 * i))
      perm_sections.push_back(i * 2);
  }
  perm_sections.push_back(knots.size());
}

int KnotColors::check_perm(int num_lines)
{
  if (knots.size() == 0)
    return false;

  const int num_knots = knots[0].size();

  // get first permutation of colours
  map<int, int> col_map;
  for (int i = 0; i < num_knots; i++) {
    map<int, int>::iterator mi = col_map.find(knots[0][i]);
    if (mi == col_map.end())
      col_map[knots[0][i]] = knots[num_lines][i];
    else {
      if (mi->second != knots[num_lines][i]) // inconsistent map of colours
        return false;
    }
  }

  // fprintf(stderr, "num_lines = %d\n", num_lines);
  // for (int i = 0; i < num_lines; i++)
  //  fprintf(stderr, "%2d -> %2d\n", i, col_map[i]);
  // fprintf(stderr, "\n");

  for (unsigned int cur_line = 0; cur_line < knots.size();
       cur_line += num_lines) {
    unsigned int next_line = (cur_line + num_lines) % knots.size();
    // fprintf(stderr, "cur=%d, next=%d\n", cur_line, next_line);
    for (int j = 0; j < num_lines; j++) {
      // fprintf(stderr, "j=%d\n", j);
      for (unsigned int k = 0; k < knots[j % 2].size(); k++) {
        int from = knots[cur_line + j][k];
        int to = knots[(next_line + j) % knots.size()][k];

        // fprintf(stderr, "cur_line=%d, j=%d, k=%d\n", cur_line, j, k);
        // fprintf(stderr, "from=%d, to=%d\n", from, to);
        // fprintf(stderr, "compare %c -> %c with %c\n", get_colour(from),
        //        get_colour(col_map[from]), get_colour(to));
        if (col_map[from] != to)
          return false;
      }
    }
  }

  // fprintf(stderr, "true\n");
  return true;
}

void KnotColors::update_symmetry()
{
  is_symmetric = true;
  if (knots.size() < 2 || knots[0].size() == knots[1].size())
    is_symmetric = false;
  else {
    for (const auto &k : knots) {
      if (!symmetric(k)) {
        is_symmetric = false;
        break;
      }
    }
  }
}

char KnotColors::get_colour(int idx) const
{
  map<int, char>::const_iterator mi = idx2col.find(idx);
  return (mi != idx2col.end()) ? mi->second : 'X';
}

int KnotColors::get_idx(char col) const
{
  map<char, int>::const_iterator mi = col2idx.find(col);
  return (mi != col2idx.end()) ? mi->second : -1;
}

int KnotColors::num_knots() const
{
  if (knots.size() > 1)
    return (knots[0].size() + knots[1].size()) / (2 * knots.size());
  else
    return 0;
}

Status KnotColors::read(FILE *ifile)
{
  vector<string> knot_lines;
  Status stat = read_knot_lines(ifile, knot_lines);
  if (stat.is_ok())
    process_knots(knot_lines);
  return stat;
}

Status KnotColors::read_knot_lines(FILE *ifile, vector<string> &knot_lines)
{
  int file_line_no = 0; // line number in the file

  char *line = 0;
  int row_num = 0; // first row will be 1
  while (read_line(ifile, &line) == 0) {
    file_line_no++;
    string data;
    for (char *p = line; *p; p++)
      if (!isspace(*p))
        data += *p;
    free(line);

    if (data == "")
      continue;

    row_num++;
    int num_knots = data.size();
    if (row_num == 1) {
      if (num_knots < 1)
        return Status::error(
            msg_str("knot line %d has %d knots, must have 1 or more", row_num,
                    num_knots));
    }
    else if (row_num == 2) {
      const int first_knots = knot_lines[0].size();
      if (num_knots < 1 || num_knots < first_knots - 1 ||
          num_knots > first_knots)
        return Status::error(
            msg_str("knot line %d has %d knots, must have %d or %d", row_num,
                    num_knots, first_knots, first_knots - 1));
    }
    else if (row_num > 2) {
      const int knots = knot_lines[(row_num + 1) % 2].size();
      if (num_knots != knots)
        return Status::error(msg_str("knot line %d has %d knots, must have %d",
                                     row_num, num_knots, knots));
    }

    knot_lines.push_back(data);
  }

  if (knot_lines.size() == 0)
    return Status::error("no knot lines given");
  else if (knot_lines.size() % 2 == 1)
    return Status::error("odd number of knot lines given");

  return Status::ok();
}

// Process the input knots: convert colour characters to colour indexes,
// get the maximum number of times each colour appears in a line of knots,
// get the index number/colour character maps
void KnotColors::process_knots(const vector<string> &knot_lines)
{
  req_colour_cnts.resize(0);
  knots.clear();
  idx2col.clear();
  col2idx.clear();

  for (unsigned int i = 0; i < knot_lines.size(); i++) {
    map<char, int> cnts;
    for (unsigned int j = 0; j < knot_lines[i].size(); j++)
      cnts[knot_lines[i][j]]++;

    for (map<char, int>::iterator mi = cnts.begin(); mi != cnts.end(); mi++) {
      map<char, int>::iterator ci = col2idx.find(mi->first);
      int idx;
      if (ci == col2idx.end()) {
        idx = req_colour_cnts.size();
        col2idx[mi->first] = idx;
        idx2col[idx] = mi->first;
        req_colour_cnts.push_back(0);
      }
      else {
        idx = ci->second;
      }

      if (mi->second > req_colour_cnts[idx])
        req_colour_cnts[idx] = mi->second;
    }

    knots.push_back(vector<int>(knot_lines[i].size()));
    for (unsigned int j = 0; j < knot_lines[i].size(); j++)
      knots.back()[j] = col2idx[knot_lines[i][j]];
  }

  num_strings = 1 + knots[0].size() + knots[1].size();
  num_colours = req_colour_cnts.size();
  num_free_strings = num_strings;
  for (int i = 0; i < num_colours; i++)
    num_free_strings -= req_colour_cnts[i];

  right.resize(num_strings / 2);
  for (int i = 0; i < num_strings / 2; i++)
    right[i] = col2idx[knot_lines[0][i]];

  fixed_strings = req_colour_cnts;
  excess_strings.resize(req_colour_cnts.size(), 0);
  for (unsigned int i = 0; i < knots[0].size(); i++) {
    int idx = knots[0][i];
    fixed_strings[idx]--;
    excess_strings[idx]++;
  }

  get_perm_sections();
  update_symmetry();
}

void KnotColors::print_knots(FILE *file) const
{
  for (unsigned int i = 0; i < knots.size(); i++) {
    int offset = i % 2;
    fprintf(file, "%3d:  ", i + 1);
    fprintf(file, " %s", offset ? "  " : "");
    for (unsigned int j = 0; j < knots[i].size(); j++)
      fprintf(file, "%c   ", get_colour(knots[i][j]));
    fprintf(file, "\n");
  }
}

// order through counts of colours that add up to the number of free strings
static int get_next_cnts(vector<int> &cnts, int free)
{
  vector<int> sums(cnts.size());
  int sum = 0;
  for (unsigned int i = 0; i < cnts.size(); i++) {
    sum += cnts[i];
    sums[i] = sum;
  }
  sums.back() = free;

  if (sum) {
    int sz = sums.size();
    sums[sz - 1] += 1;
    for (int i = 0; i < sz; i++) {
      const int idx = (sz - i) - 1;
      if (sums[idx] > free) {
        if (idx == 0) // reached maximum
          return false;
        else {
          sums[idx - 1] += 1;          // increment previous digit
          if (sums[idx - 1] <= free) { // valid, so update following digits
            for (int j = idx; j < sz - 1; j++)
              sums[j] = sums[idx - 1];
            sums.back() = free;
            break;
          }
        }
      }
    }
  }

  // print_ints(sums, "sums after");

  for (unsigned int i = 0; i < cnts.size(); i++)
    cnts[i] = (i) ? sums[i] - sums[i - 1] : sums[i];

  return true;
}

vector<Pattern> KnotColors::solve(bool prefer_pure, bool optimal)
{
  vector<Pattern> solns;
  Pattern pat;
  pat.set_prefer_pure(prefer_pure);

  vector<int> free_strings(num_colours, 0);
  while (get_next_cnts(free_strings, num_free_strings)) {
    fprintf(stderr, "\n================================\n");
    print_cnts(stderr, free_strings, *this, "Free string colour counts");

    vector<int> left((num_strings + 1) / 2);
    int pos = 0;
    for (int i = 0; i < num_colours; i++) {
      const int total = free_strings[i] + fixed_strings[i];
      for (int j = 0; j < total; j++) {
        left[pos++] = i;
      }
    }

    vector<int> strings = left;
    strings.insert(strings.end(), right.begin(), right.end());
    std::sort(strings.begin(), strings.end());

    while (true) {
      pat.set_strings(strings);
      for (unsigned int j = 0; j < pat.strings.size(); j++)
        fprintf(stderr, "%c ", get_colour(pat.strings[j]));
      fprintf(stderr, "\n");
      // if (!pat.only_symmetric || symmetric(strings)) {
      vector<Pattern> sols = pat.solve(*this, optimal);
      if (sols.size()) {
        if (solns.size() == 0 || solns[0].score > sols[0].score)
          solns = sols;
        else if (sols[0].score == sols[0].score)
          solns.insert(solns.end(), sols.begin(), sols.end());
      }
      if (solns.size())
        pat.set_score_limit(solns[0].score);
      //}

      if (!std::next_permutation(strings.begin(), strings.end()))
        break;
      pat.increment_perm();
    }

    pat.increment_strs();
    if (num_free_strings == 0)
      break;
  }
  return solns;
}

void print_cnts(FILE *file, const vector<int> &cnts, const KnotColors &k,
                const string &desc)
{
  if (desc.size())
    fprintf(file, "%s:\n", desc.c_str());
  for (unsigned int i = 0; i < cnts.size(); i++)
    fprintf(file, "   %c: %3d\n", k.get_colour(i), cnts[i]);
}

bool symmetric(const vector<int> &vec)
{
  bool is_sym = true;
  const int len = vec.size() / 2;
  const int last = (int)vec.size() - 1;
  for (int i = 0; i < len; i++)
    if (vec[i] != vec[last - i]) {
      is_sym = false;
      break;
    }

  return is_sym;
}
