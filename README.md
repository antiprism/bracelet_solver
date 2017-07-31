# README

## Friendship Bracelet Normal Pattern Solver

### bracelet_solver

bracelet_solver is a small command line utility to find an optimal
knotting pattern for a friendship bracelet design.

The program should be considered early alpha and, although it may
receive some updates, no further development has been planned. The
code is shared for interest only. See [issues](#issues).

### Install

The package uses an Autotools-based build. The repository does not
include a configure script, but this can be created by installing the
Autotools and running *autoreconf* in the top directory. Then, build as
usual with, e.g.

``` cmds
./configure
make
```
The program executables can be found in the src directory.


### Preparing the Bracelet Design File

The starting point is a bracelet design. This is a repeatable section of
a bracelet described by the colours of the knots in each line. You can make
an original design by following this
[bracelet design tutorial](http://friendship-bracelets.net/tutorial.php?id=114),
or you could modify an existing pattern from the
[Friendship-bracelets.net patterns database](http://friendship-bracelets.net/patterns2.php).

The bracelet design must be converted to a simple text format and
written to a file. This file can be prepared and edited using a text
editor.

To convert the design, assign a single character to each colour in the
bracelet (letters are case sensitive). Enter each line of knots in the
bracelet section in order as line of text, with each knot in the line
being represented by its colour character. Spaces can be used to improve
the readability.

If you wish to modify a bracelet from the
[patterns database](http://friendship-bracelets.net/patterns2.php),
you can easily copy the design into a file using the [pat2design](#pat2design)
utility.

### Example Bracelet Design Files

A simple example is a
[2-colour chevron](http://friendship-bracelets.net/pattern.php?id=2).

The design has two colours: yellow and red. Assign the character 'y' to
yellow, and 'r' to red. The design has four lines of knots, the number
of knots in the lines alternate between 4 and 3. The bracelet design file
would contain the following text (optional spaces have been added for
clarity), and can be download as [design2.txt](doc/design2.txt)

``` file
y r r y
 y r y
r y y r
 r y r
```

It would work equally well to assign '0' to red and '1' to yellow and
leave out the spaces. The file would contain the following text, and
give exactly the same results.

``` file
1001
101
0110
010
```

### Running the Program

The program is a command line utility. It reads a bracelet design file and
produces a report with information about the design and an optimal knotting
solution. For more help on the program and its options run

``` cmd
bracelet_solver -h
```

A typical way to run the program is as follows

``` cmd
bracelet_solver design2.txt > report_design2.txt
```

The program reads the design from the file *design2.txt* and writes the
report into the file *report\_design2.txt*. You can view the report file
with a text processor or word processor.

**Note**: the program algorithm is only partially optimised. It may take
a long (minutes), or *very* long (centuries?), time to find an optimal
solution for larger designs or those with few colours.

### Solution Report

#### Text pattern format

The numbered knot lines are sandwiched above and below by strings.
The two strings above a knot lead into the knot, and then exit as the
two strings below. If the two strings are the same colour use any knot
type to tie the knot. Otherwise, if the top left (right) string colour
matches the knot colour the first half o the knot is a forward (backward),
and if the bottom left (right) string matches the knot colour the second
half of the knot is a backward (forward).

Example text pattern output

``` file
     y r r y y r r y
 1:   y   r   r   y 
     r y y r r y y r
 2:     y   r   y   
     r y y r r y y r
 3:   r   y   y   r  
     y r r y y r r y
 4:     r   y   r   
     y r r y y r r y
```

#### Generator pattern format

Use this with the
[Friendship-bracelets.net Alternative Generator](http://friendship-bracelets.net/js_gen.php)
Add more or less rows or strings to match the knot rows and strings in
the solution pattern. Set up the string colours to match the input
strings. Click the "Show console" box. Select and copy the knot text
(don't include the string line) from the solution, then select the text
in the console box and paste in the solution, replacing the original knot
text. Add a string and then remove a string to refresh the displayed
pattern with the new solution knots.

Example generator pattern output

``` file
y r r y y r r y 

DDdd
DDD
DDdd
DDD
```
### Program Help

The program help can be printed with *bracelet\_solver -h*.

``` file
Usage: bracelet_solver [options] bracelet_design_file

Read a text file containing a bracelet design, a formatted text
representation of a repeating section of a bracelet, and find
sets of strings and knotting patterns that will produce this section.
Produce a report of solutions and write it to standard output

bracelet_design_file
A text file holding the knot lines and colours for a repeating section
of a bracelet, in the following simple format. Assign a single character
to each colour in the bracelet (letters are case sensitve). Enter each
line of knots in the bracelet section in order as line of text, with each
knot in the line represented by its colour character. The final text file
will have an even number of lines, and as many lines as there are lines
of knots in the bracelet section. If the first line has N knots, either
all the lines will have N knots, or the lines will alternate between
N and N-1 knots. Not all patterns have solutions!

Options
  -h         print this help text
  -v         print the program version and licence
  -m <type>  find solution with most mixed knots (default: most pure knots)
  -s         only symmetric solutions (except middle line)
```

### <a name="issues"></a>Issues

 * Program has not been sufficiently tested
 * Algorithm very slow on some designs, needs further optimisation.
 * Option -s is not fully implemented (e.g. generator pattern not symmetric)

Useful improvements:
 * Some designs can be viewed as a shorter design that permutes colours. These
   shorter designs could be solved directly, with the corresponding knotting
   pattern permuting the outgoing string colours in the same way. (Should
   be straightforward to implement).
 * Review choice of knot when same colour strings meet (maybe prefer long
   runs of the same forward/backward type, prefer symmetric where
   possible, etc.)


### <a name="pat2design"></a>pat2design

*pat2design* is a very basic utility for converting a published pattern
into a bracelet design input file for *bracelet\_solver*. This makes it
easier to modify an existing pattern, as it saves having to convert the
pattern by hand.

Make an input pattern file, in exactly the same format as the generator
pattern text produced in *bracelet\_solver* report. This is a line of
characters to represent the string colours, followed by a knotting pattern.
The knotting pattern is copy pasted from the Friendship-bracelets.net
Alternative Generator console.

Here is an example of how to convert Pattern \#2. Start up the
[Alternative Generator with Pattern \#2](http://friendship-bracelets.net/js_gen.php?edit=2)
(for a different pattern replace the 2 in the URL with the number of
the pattern you wish to modify).

Write the top line of strings as a character for each colour, e.g.

``` file
yrryyrry
```

Click on "Show console", and copy the text into the file below the
strings line. The final file, which can be downloaded as
[pat2.txt](doc/pat2.txt), contains the following text

``` file
yrryyrry
DDdd
DDd
DDdd
DDd
```

*pat2design* accepts the input pattern file as an argument and writes the design file to standard output, so run it like this

``` cmd
pat2design pat2.txt > design2.txt
```

*design2.txt* contains the following text

``` file
y r r y
 y r y
r y y r
 r y r
```

This can then be modified and used as input for *bracelet\_solver* to find knotting patterns.

