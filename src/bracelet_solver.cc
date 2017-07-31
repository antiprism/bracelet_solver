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
#include "programopts.h"
#include "utils.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

class BrOpts : public ProgramOpts {
public:
  bool prefer_pure;
  string test_strings;
  bool only_symmetric;
  string ifile;

  BrOpts()
      : ProgramOpts("bracelet_solver"), prefer_pure(true), only_symmetric(false)
  {
  }
  void process_cmd_line(int argc, char **argv);
  void usage();
};

void BrOpts::usage()
{
  fprintf(
      stdout,
      "\n"
      "Usage: %s [options] bracelet_design_file\n"
      "\n"
      "Read a text file containing a bracelet design, a formatted text\n"
      "representation of a repeating section of a bracelet, and find\n"
      "sets of strings and knotting patterns that will produce this section.\n"
      "Produce a report of solutions and write it to standard output\n"
      "\n"
      "bracelet_design_file\n"
      "A text file holding the knot lines and colours for a repeating section\n"
      "of a bracelet, in the following simple format. Assign a single "
      "character\n"
      "to each colour in the bracelet (letters are case sensitve). Enter each\n"
      "line of knots in the bracelet section in order as line of text, with "
      "each\n"
      "knot in the line represented by its colour character. The final text "
      "file\n"
      "will have an even number of lines, and as many lines as there are "
      "lines\n"
      "of knots in the bracelet section. If the first line has N knots, "
      "either\n"
      "all the lines will have N knots, or the lines will alternate between\n"
      "N and N-1 knots. Not all patterns have solutions!\n"
      "\n"
      "Options\n"
      "  -h         print this help text\n"
      "  -v         print the program version and licence\n"
      "  -m <type>  find solution with most mixed knots (default: most pure "
      "knots)\n"
      "  -s         only symmetric solutions (except middle line)\n"
      "\n"
      "\n",
      prog_name());
}

void BrOpts::process_cmd_line(int argc, char **argv)
{
  opterr = 0;
  char c;

  handle_long_opts(argc, argv);

  while ((c = getopt(argc, argv, ":hvms")) != -1) {
    if (common_opts(c, optopt))
      continue;

    switch (c) {
    case 'm':
      prefer_pure = false;
      break;

    case 's':
      only_symmetric = true;
      break;

    default:
      error("unknown command line error");
    }
  }

  int num_args = argc - optind;
  if (num_args != 1)
    error(msg_str("%s knot colour file specified",
                  (num_args == 0) ? "no" : "more than one"),
          "bracelet_design_file");

  ifile = argv[optind];
}

Status print_header(const KnotColors &k, string prog_name)
{
  fprintf(stdout,
          "The following knotting solutions were found by\n"
          "\n"
          "   %s\n"
          "   http://www.antiprism.com/other/bracelet_solver\n"
          "\n"
          "If you publish any of these solutions then please acknowledge the\n"
          "program and include a reference to the web page.\n"
          "\n",
          prog_name.c_str());

  fprintf(stdout, "Knot pattern to solve:\n");
  k.print_knots(stdout);
  fprintf(stdout, "\n");

  fprintf(stdout, "Number of colours: %3d\n", k.num_colours);
  fprintf(stdout, "Number of strings: %3d\n", k.num_strings);
  // The number of knots of each colour in a line puts a limit on the minimum
  // number strings of that colour required. The total number of required
  // strings is subtracted from the number of strings in the pattern to give
  // the number of free strings. The free strings can be assigned various
  // combinations of colours, then combined with the required strings and
  // used to search for solutions.
  //   fprintf(stdout, "Number of free strings: %3d\n", k.num_free_strings);

  fprintf(stdout, "\n");

  print_cnts(stdout, k.req_colour_cnts, k, "Required string colour counts");

  const char free_strings_msg[] =
      "pattern cannot be made.\n"
      "The number of knots of each colour in a line puts a limit on the "
      "minimum\n"
      "number strings of that colour required, and the total number of "
      "required\n"
      "strings exceeds the number of strings in the proposed pattern.";
  if (k.num_free_strings < 0) {
    fprintf(stdout, "\n%s\n\n", free_strings_msg);
    return Status::error(free_strings_msg);
  }

  return Status::ok();
}

void print_solutions(const vector<Pattern> &solns, const KnotColors &k)
{
  int sol_no = 0;
  for (const auto &pat : solns)
    pat.print_pattern(k, sol_no++, stdout);
}

int main(int argc, char **argv)
{
  BrOpts opts;
  opts.process_cmd_line(argc, argv);
  FILE *file;
  if (opts.ifile == "" || opts.ifile == "-")
    file = stdin;
  else {
    if (!(file = fopen_file(opts.ifile)))
      opts.error(
          msg_str("could not open knot colours file '%s'", opts.ifile.c_str()));
  }

  KnotColors k;
  opts.print_status_or_exit(k.read(file));

  bool only_symmetric = opts.only_symmetric;
  if (only_symmetric && !k.is_symmetric) {
    opts.warning("design is not symmetric, option ignored", 's');
    only_symmetric = false;
  }

  opts.print_status_or_exit(print_header(k, opts.prog_name()));
  vector<Pattern> solns = k.solve(opts.prefer_pure, false);
  print_solutions(solns, k);

  return 0;
}
