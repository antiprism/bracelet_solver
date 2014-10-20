
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <climits>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

char version[] = "0.01";
char prog_name[] = "bracelet_solver";
const size_t MSG_SZ = 256;

void message(string msg, const char *msg_type, string opt)
{
   fprintf(stderr, "%s: ", prog_name);
   if(msg_type)
      fprintf(stderr, "%s: ", msg_type);
   if(opt != "") {
     if(opt.size()==1 || opt[0]=='\0') 
        fprintf(stderr, "option -%s: ", opt.c_str());
     else
        fprintf(stderr, "%s: ", opt.c_str());
   }

   fprintf(stderr, "%s\n", msg.c_str());
}

void error(string msg, string opt="")
{
   message(msg, "error", opt);
   exit(1);
}

void error(string msg, char opt) { error(msg, string()+opt); }

bool read_int(const char *str, int *i, char *errmsg=0)
{
   char buff;
   if( sscanf(str, " %d %c", i,  &buff) != 1) {
      if(errmsg)
         strcpy(errmsg, "not an integer");
      return false;
   }

   if(*i==INT_MAX) {
      if(errmsg)
         sprintf(errmsg, "integer too large\n");
      return false;
   }

   return true;
}


FILE *fopen_file(string &fpath)
{
   FILE *file=fopen(fpath.c_str(), "r");
   if(file) {
      struct stat st;
      fstat(fileno(file), &st);
      if(S_ISDIR(st.st_mode)) {
         fclose(file);
         file = 0;
      }
   }
   return file;
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
         if(offset != 0)
            return 0;
         else {
            *(*line+offset) = '\0'; // terminate the line
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


class knot_colours
{
   public:
      vector<vector<int> > knots;

      vector<int> req_colour_cnts;
      map<char, int> col2idx;
      map<int, char> idx2col;

      vector<int> fixed_strings;
      vector<int> excess_strings;
      vector<int> right;

      int num_strings;
      int num_colours;
      int num_free_strings;

      int read_knot_lines(FILE *ifile, vector<string> &knot_lines, char *msg);
      void process_knots(const vector<string> &knot_lines);
      char get_colour(int idx) const;
      int get_idx(char col) const;

      int read(FILE *ifile, char *msg);

      void print_knots(FILE *file) const;
};


char knot_colours::get_colour(int idx) const
{
   map<int, char>::const_iterator mi = idx2col.find(idx);
   return (mi!=idx2col.end()) ? mi->second : 'X';
}

int knot_colours::get_idx(char col) const
{
   map<char, int>::const_iterator mi = col2idx.find(col);
   return (mi!=col2idx.end()) ? mi->second : -1;
}


int knot_colours::read(FILE *ifile, char *msg)
{
   *msg = '\0';
   vector<string> knot_lines;
   if(!read_knot_lines(ifile, knot_lines, msg))
      return false;

   process_knots(knot_lines);
   return true;
}


int knot_colours::read_knot_lines(FILE *ifile, vector<string> &knot_lines,
      char *msg)
{
   *msg = '\0';
   int file_line_no = 0; // line number in the file

   char *line = 0;
   int row_num = 0;   // first row will be 1
   while(read_line(ifile, &line)==0) {
      file_line_no++;
      string data;
      for(char *p=line; *p; p++)
         if(!isspace(*p))
            data += *p;
      free(line);

      if(data=="")
         continue;

      row_num++;
      int num_knots = data.size();
      if(row_num==1) {
         if(num_knots < 1) {
            snprintf(msg, MSG_SZ,
                  "knot line %d has %d knots, must have 1 or more",
                  row_num, num_knots);
            return false;
         }
      }
      else if(row_num==2) {
         const int first_knots = knot_lines[0].size();
         if( num_knots < 1 || num_knots < first_knots-1 ||
               num_knots > first_knots ) {
            snprintf(msg, MSG_SZ,
                  "knot line %d has %d knots, must have %d or %d",
                  row_num, num_knots, first_knots, first_knots-1);
            return false;
         }
      }
      else if(row_num > 2) {
         const int knots = knot_lines[(row_num+1)%2].size();
         if(num_knots != knots) {
            snprintf(msg, MSG_SZ,
                  "knot line %d has %d knots, must have %d",
                  row_num, num_knots, knots);
            return false;
         }
      }

      knot_lines.push_back(data);

   }

   if(knot_lines.size()==0) {
      snprintf(msg, MSG_SZ, "no knot lines given");
      return false;
   }
   else if(knot_lines.size()%2==1) {
      snprintf(msg, MSG_SZ, "odd number of knot lines given");
      return false;
   }

   return true;
}


// Process the input knots: convert colour characters to colour indexes,
// get the maximum number of times each colour appears in a line of knots,
// get the index number/colour character maps
void knot_colours::process_knots(const vector<string> &knot_lines)
{
   req_colour_cnts.resize(0);
   knots.clear();
   idx2col.clear();
   col2idx.clear();

   for(unsigned int i=0; i<knot_lines.size(); i++) {
      map<char, int> cnts;
      for(unsigned int j=0; j<knot_lines[i].size(); j++)
         cnts[knot_lines[i][j]]++;

      for(map<char, int>::iterator mi=cnts.begin(); mi!=cnts.end(); mi++) {
         map<char, int>::iterator ci = col2idx.find(mi->first);
         int idx;
         if(ci==col2idx.end()) {
            idx = req_colour_cnts.size();
            col2idx[mi->first] = idx;
            idx2col[idx] = mi->first;
            req_colour_cnts.push_back(0);
         } else {
            idx = ci->second;
         }

         if(mi->second > req_colour_cnts[idx])
            req_colour_cnts[idx] = mi->second;
      }

      knots.push_back(vector<int>(knot_lines[i].size()));
      for(unsigned int j=0; j<knot_lines[i].size(); j++)
         knots.back()[j] = col2idx[knot_lines[i][j]];
   }

   num_strings = 1 + knots[0].size()+knots[1].size();
   num_colours = req_colour_cnts.size();
   num_free_strings = num_strings;
   for(int i=0; i<num_colours; i++)
      num_free_strings -= req_colour_cnts[i];

   right.resize(num_strings/2);
   for(int i=0; i<num_strings/2; i++)
      right[i] = col2idx[knot_lines[0][i]];

   fixed_strings = req_colour_cnts;
   excess_strings.resize(req_colour_cnts.size(), 0);
   for(unsigned int i=0; i<knots[0].size(); i++) {
      int idx = knots[0][i];
      fixed_strings[idx]--;
      excess_strings[idx]++;
   }

}

void knot_colours::print_knots(FILE *file) const
{
   for(unsigned int i=0; i<knots.size(); i++) {
      int offset = i%2;
      fprintf(file, "%3d:  ", i+1);
      fprintf(file, " %s", offset ? "  " : "");
      for(unsigned int j=0; j<knots[i].size(); j++)
         fprintf(file, "%c   ", get_colour(knots[i][j]));
      fprintf(file, "\n");
   }
}



void print_cnts(FILE *file, const vector<int> &cnts,
      const knot_colours &k, const string &desc)
{
   if(desc.size())
      fprintf(file, "%s:\n", desc.c_str());
   for(unsigned int i=0; i<cnts.size(); i++)
         fprintf(file, "   %c: %3d\n", k.get_colour(i), cnts[i]);
}



// order through counts of colours that add up to the number of free strings
int get_next_cnts(vector<int> &cnts, int free)
{
   vector<int> sums(cnts.size());
   int sum=0;
   for(unsigned int i=0; i<cnts.size(); i++) {
      sum += cnts[i];
      sums[i] = sum;
   }
   sums.back() = free;

   if(sum) {
      int sz = sums.size();
      sums[sz-1] += 1;
      for(int i=0; i<sz; i++) {
         const int idx = (sz-i) - 1;
         if(sums[idx]>free) {
            if(idx==0)                     // reached maximum
               return false;
            else {
               sums[idx-1] += 1;           // increment previous digit
               if(sums[idx-1]<=free) {     // valid, so update following digits
                  for(int j=idx; j<sz-1; j++)
                     sums[j] = sums[idx-1];
                  sums.back() = free;
                  break;
               }
            }
         }
      }
   }

   //print_ints(sums, "sums after");

   for(unsigned int i=0; i<cnts.size(); i++)
      cnts[i] = (i) ? sums[i] - sums[i-1] : sums[i];

   return true;
}

int increment(vector<int> &digits)
{
   int sz = digits.size();
   digits[sz-1] += 1;
   for(int i=0; i<sz; i++) {
      const int idx = (sz-i) - 1;
      if(digits[idx]>1) {
         if(idx==0)                     // reached maximum
            return false;
         else {
            digits[idx-1] += 1;           // increment previous digit
            if(digits[idx-1]<=1) {     // valid, so update following digits
               for(int j=idx; j<sz-1; j++)
                  digits[j] = 0;
               digits.back() = 0;
               break;
            }
         }
      }
   }
   return true;
}

vector<int> get_next_strings(const vector<int> &strings_in,
      const vector<int> &knots, int offset, int *redundant)
{
   if(redundant)
      *redundant = false;
   vector<int> strings_out = strings_in;
   for(unsigned int i=0; i<knots.size(); i++) {
      const int idx = offset+2*i;
      if(knots[i]) {
         // only count solutions that don't swap strings of the same colour
         if(redundant && strings_out[idx]==strings_out[idx+1]) {
            *redundant = true;
            break;
         }
         std::swap(strings_out[idx], strings_out[idx+1]);
      }
   }

   return strings_out;
}



class pattern
{
   public:
      int soln_no;                 // for all solutions
      int soln_strs_no;            // for a set of free colours
      int soln_perm_no;            // for a permutation of free colours
      int soln_var_no;             // for knotting variation of a permutation

      int max_soln_number;         // finish if solution number is this
      int max_soln_var_number;     // limit for number of knotting variations

      vector<int> strings;         // Original start strings
      vector<int> start_strings;   // Start strings adjusted for wrapping
      vector<vector<int> > knots;  // Knots (swap strings or not)
      vector<int> start_knots;     // First line adjusted for new start strings

      int report_level;            // 0:Strings, 1:Perms, 2:Knot Vars
      bool reorder;                // Reorder start strings to match end strings
      long descent_cnt;            // Number of recursion descents

      pattern();
      int set_strings(const vector<int> &left, const vector<int> &right);
      int set_strings(const vector<int> &strs);
      void set_max_soln_number(int max) { max_soln_number=max; }
      void set_max_soln_var_number(int max) { max_soln_var_number=max; }
      void set_reorder(bool reord) { reorder = reord; }
      void set_report_level(int lvl) { report_level = lvl; }
      void increment_strs()
         { soln_strs_no++; soln_perm_no=0; soln_var_no=0; }
      void increment_perm()
         { soln_perm_no++; soln_var_no=0; }
      void increment_var()
         { soln_var_no++; soln_no++; }
      int done()
         { return (max_soln_number && soln_no>=max_soln_number); }
      int done_vars()
         { return (max_soln_var_number && soln_var_no>=max_soln_var_number); }

      int check_line(int line, const vector<int> &strs,const knot_colours &k);
      int solve(const knot_colours &k);
      string get_strings_text(map<int, char> &idx2col);

      void print_strings_and_knots(const knot_colours &k);
      void print_generator_pattern(const knot_colours &k);
};

pattern::pattern():
   soln_no(0), soln_strs_no(0), soln_perm_no(0), soln_var_no(0),
   max_soln_number(0), max_soln_var_number(0),
   report_level(0), reorder(true), descent_cnt(0)
{
}


void pattern::print_strings_and_knots(const knot_colours &k)
{
   vector<int> str = start_strings;
   for(unsigned int i=0; i<=k.knots.size(); i++) {
      int offset = i%2;
      fprintf(stdout, "      ");
      for(unsigned int j=0; j<str.size(); j++)
         fprintf(stdout, "%c ", k.get_colour(str[j]));
      fprintf(stdout, "\n");
      if(i<k.knots.size()) {
         fprintf(stdout, "%3d:  ", i+1);
         fprintf(stdout, " %s", offset ? "  " : "");
         for(unsigned int j=0; j<k.knots[i].size(); j++)
            fprintf(stdout, "%c   ", k.get_colour(k.knots[i][j]));
         fprintf(stdout, "\n");
         const vector<int> &knts = (i) ? knots[i] : start_knots;
         vector<int> str_tmp = get_next_strings(str, knts, offset, 0);
         str = str_tmp;
      }
      else
         str = start_strings;
   }
}


void pattern::print_generator_pattern(const knot_colours &k)
{
   vector<int> str = start_strings;
   for(unsigned int i=0; i<str.size(); i++)
      fprintf(stdout, "%c ", k.get_colour(str[i]));
   fprintf(stdout, "\n\n");
   for(unsigned int i=0; i<k.knots.size(); i++) {
      int offset = i%2;
      const vector<int> &knts = (i) ? knots[i] : start_knots;
      vector<int> str_next = get_next_strings(str, knts, offset, 0);
      //for(unsigned int s=0; s<str.size(); s++)
      //   fprintf(stdout, "%d ", str[s]);
      //fprintf(stdout, "\n");
      //for(unsigned int s=0; s<str_next.size(); s++)
      //  fprintf(stdout, "%d ", str_next[s]);
      //fprintf(stdout, "\n");
      for(unsigned int j=0; j<k.knots[i].size(); j++) {
         int K = k.knots[i][j];
         int TL = str[2*j+offset];
         //int TR = str[2*j+offset+1];
         //int BL = str_next[2*j+offset];
         int BR = str_next[2*j+offset+1];
         bool switch_strs = (TL==BR);
         bool knot_left_str = (TL==K);
         char knot_types[] = "RrdD";
         int idx = 2*switch_strs+knot_left_str;
         //fprintf(stdout, "%sswitch, knot %s -> type[%d]='%c'\n",
         //      switch_strs?"": "don't ", knot_left_str?"left":"right",
         //      idx, knot_types[idx]);
         //fprintf(stdout,"[%d,%d](%d,%d) = (%d,%d [%d] %d,%d)\n", i, j, 2*j+offset, 2*j+offset+1, TL, TR, K, BL, BR);
         fprintf(stdout, "%c", knot_types[idx]);
      }
      str = str_next;
      fprintf(stdout, "\n");
   }
}



int pattern::set_strings(const vector<int> &left, const vector<int> &right)
{
   if(left.size()!=right.size() && left.size()!=right.size()+1)
      return false;
   strings.resize(left.size() + right.size());
   for(unsigned int i=0; i<right.size(); i++) {
      strings[2*i]   = left[i];
      strings[2*i+1] = right[i];
   }
   if(left.size()>right.size())
      strings.back() = left.back();
   return true;
}

int pattern::set_strings(const vector<int> &strs)
{
   strings = strs;
   return true;
}

int pattern::check_line(int line, const vector<int> &strings_in,
      const knot_colours &k)
{
   //Knots either switch the input strings or leave them as they are.
   //Permute through all knot possibilities for this line. If output
   //strings are able to make the line below, recurse.
   descent_cnt++;
   if(!reorder && descent_cnt%10000==0) {
      fprintf(stderr, ".");
      if(descent_cnt%100000==0)
         fprintf(stderr, " ");
      if(descent_cnt%700000==0)
         fprintf(stderr, "\n");
      fflush(stderr);
   }

   // knots is effectively a binary numbery, initialise to 0
   fill(knots[line].begin(), knots[line].end(), 0);

   const int first = line%2;    // first string that will be knotted
   while(true) {
      // Pairs of input strings may be switched or left by the knots
      int redundant;
      vector<int> strings_out = get_next_strings(strings_in, knots[line],
            first, &redundant);

      bool valid = !redundant;

      // check each knot in the next line (wrap) is met by at least one
      // string of the same colour
      const int next_line = (line < (int)knots.size()-1) ? line+1 : 0;
      int next_first = !first;
      for(unsigned int i=0; i<k.knots[next_line].size(); i++) {
         if(k.knots[next_line][i]!= strings_out[next_first+2*i] &&
            k.knots[next_line][i]!= strings_out[next_first+2*i+1]) {
            valid = false;
            break;
         }
      }

      // Check whether strings match in pairs when wrapping to beginning
      if(valid && !next_line) {
         if(reorder) {
            for(unsigned int i=0; i<strings.size()/2; i++) {
               if( (strings_out[2*i]   == strings[2*i] &&
                        strings_out[2*i+1] == strings[2*i+1]) ||
                     (strings_out[2*i]   == strings[2*i+1] &&
                      strings_out[2*i+1] == strings[2*i]))
                  continue;   // strings match
               else {
                  valid = false;
                  break;
               }
            }

            // check last string if number of strings is odd
            if(strings.size()%2 && strings_out.back() != strings.back())
               valid = false;
         }
         else {
            valid = (strings_out==strings);
         }
      }

      if(valid) {
         if(!next_line) {     // finished
            // set the starting strings to the final strings,
            start_strings = strings_out;
            // and adjust the first line of knots
            start_knots = knots[0];
            if(reorder) {
               for(unsigned int i=0; i<knots[0].size(); i++) {
                  if(strings[2*i] != strings[2*i+1] &&
                        start_strings[2*i] != strings[2*i])
                     start_knots[i] = !start_knots[i];
               }
            }
         }
         else               // check the next line
            valid = check_line(line+1, strings_out, k);
      }


      if(valid) {
         if(report_level<1 && !soln_perm_no && !soln_var_no) {
            fprintf(stdout, "\nSTRINGS NO.: %d\n", soln_strs_no+1);
            fprintf(stdout, "fixed strings    :  ");
            for(unsigned int j=0; j<strings.size(); j++)
               if(j%2==1)
                  fprintf(stdout, " %c", k.get_colour(strings[j]));
            fprintf(stdout, "\n");
            fprintf(stdout, "permuted strings : ");
            for(unsigned int j=0; j<strings.size(); j++)
               if(j%2==0)
                  fprintf(stdout, " %c", k.get_colour(strings[j]));
            fprintf(stdout, "\n");
         }
         if(report_level<2 && !soln_var_no) {
            fprintf(stdout, "\nPERMUTATION NO.: %d\n", soln_perm_no+1);
            fprintf(stdout, "fixed strings :  ");
            for(unsigned int j=0; j<strings.size(); j++)
               if(j%2==1)
                  fprintf(stdout, " %c", k.get_colour(strings[j]));
            fprintf(stdout, "\n");
            fprintf(stdout, "permution     : ");
            for(unsigned int j=0; j<strings.size(); j++)
               if(j%2==0)
                  fprintf(stdout, " %c", k.get_colour(strings[j]));
            fprintf(stdout, "\n");
         }

         fprintf(stdout, "\nKNOTTING VARIATION: %d   ", soln_var_no+1);
         if(report_level<2)
            fprintf(stdout, "(strings no.:%4d   permutation no.:%4d)",
               soln_strs_no+1, soln_perm_no+1);
         fprintf(stdout, "\n");
         fprintf(stdout, "solution no. : %d\n", soln_no+1);
         fprintf(stdout, "base strings: ");
         for(unsigned int j=0; j<strings.size(); j++)
            fprintf(stdout, "%c", k.get_colour(strings[j]));
         if(strings != start_strings)
            fprintf(stdout, " (reordered)");
         fprintf(stdout, "\n\n");
         fprintf(stdout, "Text pattern format:\n\n");
         print_strings_and_knots(k);
         fprintf(stdout, "\n");
         fprintf(stdout, "Generator pattern format:\n\n");
         print_generator_pattern(k);
         fprintf(stdout, "\n");
         increment_var();
      }

      if(done() || done_vars())
         break;

      if(!increment(knots[line]))
         break;

   }

   return false; // none of the knot permutations for this line were valid
}

int pattern::solve(const knot_colours &k)
{
   //initialise knots
   knots.clear();
   knots.resize(k.knots.size());
   for(unsigned int i=0; i<k.knots.size(); i++)
      knots[i].resize(k.knots[i].size());

   return check_line(0, strings, k);
}

string pattern::get_strings_text(map<int, char> &idx2col)
{
   string txt;
   const int sz = strings.size();
   for(int i=0; i<sz; i++) {
      txt += idx2col[strings[i]];
      if(i < sz-1)
         txt += " ";
   }

   return txt;
}

struct options {
   int max_soln_number;
   int max_soln_var_number;
   string test_strings;
   string ifile;

   options():
      max_soln_number(1000),
      max_soln_var_number(1)
      {}
   void process_cmd_line(int argc, char **argv);
};


void print_help()
{
   fprintf(stdout,
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
"of a bracelet, in the following simple format. Assign a single character\n"
"to each colour in the bracelet (letters are case sensitve). Enter each\n"
"line of knots in the bracelet section in order as line of text, with each\n"
"knot in the line represented by its colour character. The final text file\n"
"will have an even number of lines, and as many lines as there are lines\n"
"of knots in the bracelet section. If the first line has N knots, either\n"
"all the lines will have N knots, or the lines will alternate between\n"
"N and N-1 knots. Not all patterns have solutions!\n"
"\n"
"Options\n"
"  -h         print this help text\n"
"  -v         print the program version and licence\n"
"  -n <num>   maximum number of solutions to find, 0 for no limit\n"
"             (default: %d)\n"
"  -k <num>   maximum number of knotting variations to find for each\n"
"             string permutation, 0 for no limit (default: 1)\n"
"  -s <strs>  find all exact solutions for a specific set of strings (might\n"
"             take some time!)\n"
"\n"
"\n", prog_name, options().max_soln_number);
}

void print_version()
{
   fprintf(stdout,
"%s %s\n"
"\n"
"Copyright 2013 Adrian Rossiter <adrian@antiprism.com>\n"
"               http://www.antiprism.com/other/bracelet_solver.html\n"
"\n"
"Permission is hereby granted, free of charge, to any person obtaining a\n"
"copy of this software and associated documentation files (the 'Software'),\n"
"to deal in the Software without restriction, including without limitation\n"
"the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
"and/or sell copies of the Software, and to permit persons to whom the\n"
"Software is furnished to do so, subject to the following conditions:\n"
"\n"
"   The above copyright notice and this permission notice shall be included\n"
"   in all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS\n"
"IN THE SOFTWARE.\n",
   prog_name, version);
}


void options::process_cmd_line(int argc, char **argv)
{
   opterr = 0;
   char c;
   char errmsg[MSG_SZ];

   while( (c = getopt(argc, argv, ":hvn:k:s:")) != -1 ) {
      switch(c) {
         case 'h':
            print_help();
            exit(0);
            break;

         case 'v':
            print_version();
            exit(0);
            break;

         case 'n':
            if(!read_int(optarg, &max_soln_number, errmsg))
               error(errmsg, c);
            if(max_soln_number == 0)
               max_soln_number = INT_MAX;
            if(max_soln_number < 0)
               error("maximum number of solutions must be 1 or more, or 0 for no limit", c);
            break;

         case 'k':
            if(!read_int(optarg, &max_soln_var_number, errmsg))
               error(errmsg, c);
            if(max_soln_var_number == 0)
               max_soln_var_number = INT_MAX;
            if(max_soln_var_number < 0)
               error("maximum number of knotting variations must be 1 or more, or 0 for no limit", c);
            break;

         case 's':
            test_strings = optarg;
            break;

         default:
            error("unknown command line error");
      }
   }

   int num_args = argc-optind; 
   if(num_args != 1) {
      sprintf(errmsg, "%s knot colour file specified, see '%s -h'",
            (num_args==0)?"no":"more than one", prog_name);
      error(errmsg);
   }

   ifile = argv[optind];
}

void print_header(const knot_colours &k)
{
   fprintf(stdout,
"The following knotting solutions were found by\n"
"\n"
"   %s\n"
"   http://www.antiprism.com/other/bracelet_solver\n"
"\n"
"If you publish any of these solutions then please acknowledge the\n"
"program and include a reference to the web page.\n"
"\n", prog_name);

   fprintf(stdout, "Knot pattern to solve:\n");
   k.print_knots(stdout);
   fprintf(stdout, "\n");

   fprintf(stdout, "Number of colours:      %3d\n", k.num_colours);
   fprintf(stdout, "Number of strings:      %3d\n", k.num_strings);
//The number of knots of each colour in a line puts a limit on the minimum
//number strings of that colour required. The total number of required
//strings is subtracted from the number of strings in the pattern to give
//the number of free strings. The free strings can be assigned various
//combinations of colours, then combined with the required strings and
//used to search for solutions.
//   fprintf(stdout, "Number of free strings: %3d\n", k.num_free_strings);

   fprintf(stdout, "\n");

   print_cnts(stdout, k.req_colour_cnts, k, "Required string colour counts");

   const char free_strings_msg[] = "pattern cannot be made.\n"
"The number of knots of each colour in a line puts a limit on the minimum\n"
"number strings of that colour required, and the total number of required\n"
"strings exceeds the number of strings in the proposed pattern.";
   if(k.num_free_strings<0) {
      fprintf(stdout, "\n%s\n\n", free_strings_msg);
      error(free_strings_msg);
   }
}

int check_strings(string strings, pattern pat, const knot_colours &k,
      char *errmsg)
{
   if(strings.size()!=(unsigned int)k.num_strings) {
      sprintf(errmsg, "%d strings specified, pattern needs %d strings",
            strings.size(), k.num_strings);
      return false;
   }

   string invalid_cols;
   vector<int> strs(strings.size());
   for(unsigned int i=0; i<strings.size(); i++) {
      int idx = k.get_idx(strings[i]);
      if(idx>=0)
         strs[i] = idx;
      else
         invalid_cols += strings[i];
   }

   if(invalid_cols!="") {
      sort(invalid_cols.begin(), invalid_cols.end());
      unique(invalid_cols.begin(), invalid_cols.end());
      sprintf(errmsg, "strings include colours not used in any knot: %s",
            invalid_cols.c_str());
      return false;
   }

   fprintf(stdout, "\n\nChecking strings: %s\n", strings.c_str());

   pat.set_strings(strs);
   pat.set_report_level(2);
   pat.set_reorder(false);
   pat.solve(k);
   if(!pat.soln_no) {
      fprintf(stderr, "\n\n");  // whitespace after progress counter
      strcpy(errmsg, "pattern cannot be made.\n"
"All possible knotting variations were tried and none produced a repeating\n"
"pattern\n");
      fprintf(stdout, "\n%s\n\n", errmsg);
      return false;
   }

   return true;
}

int main(int argc, char **argv)
{
   options opts;
   opts.process_cmd_line(argc, argv);

   char errmsg[MSG_SZ];
   FILE *file;
   if(opts.ifile=="" || opts.ifile == "-")
      file = stdin;
   else {
      if(!(file=fopen_file(opts.ifile))) {
         snprintf(errmsg, MSG_SZ, "could not open knot colours file '%s'\n",
               opts.ifile.c_str());
         error(errmsg);
      }
   }

   knot_colours k;
   if(!k.read(file, errmsg))
      error(errmsg);

   pattern pat;
   pat.set_max_soln_number(opts.max_soln_number);
   pat.set_max_soln_var_number(opts.max_soln_var_number);

   print_header(k);

   if(opts.test_strings!="") {
      if(!check_strings(opts.test_strings, pat, k, errmsg))
         error(errmsg, 's');
      return 0;
   }
/*
   print_cnts(stdout, k.fixed_strings, k, "Fixed string colour counts");
   print_cnts(stdout, k.excess_strings, k, "Excess string colour counts");

*/

   vector<int> free_strings(k.num_colours, 0);
   while(get_next_cnts(free_strings, k.num_free_strings)) {
      fprintf(stdout, "\n================================\n");
      print_cnts(stdout, free_strings, k, "Free string colour counts");

      vector<int> left((k.num_strings+1)/2);
      int pos = 0;
      for(int i=0; i<k.num_colours; i++) {
         const int total = free_strings[i] + k.fixed_strings[i];
         for(int j=0; j<total; j++) {
            left[pos++] = i;
         }
      }

      while(true) {
         pat.set_strings(left, k.right);
         pat.solve(k);
         if(pat.done())
            break;

         if(!std::next_permutation(left.begin(), left.end()))
            break;
         pat.increment_perm();
      }
      if(pat.done())
         break;

      pat.increment_strs();
   }

   return 0;
}


