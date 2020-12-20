# Sudoku puzzle creator and solver

This program creates or solves Sudoku puzzles.

It is developed on GNU/Linux using gcc (but works with clang too). It only
consists of two source files: sudoku.c and sudoku.h.

## Installation

To compile it simply run make.

## Usage

It's all done from the command line.

    ./sudoku [options]

Options are:

--create (or -c) <hardness>

Creates a puzzle. Argument of 0 is easiest. 2 or higher is very hard. The
puzzles are randomly generated.

--easy (or -e) <blanks>

Creates easier puzzles than the -c option. The *blanks* argument specifies the
number of blank squares the puzzle should have. The higher this number, generally,
the harder the puzzle. Setting it too high (e.g. 60) can result in behaviour
indistinguishable from an endless loop. Setting it to 70 definitely generates an
endless loop. Setting it to 0 is quick but the number of blanks will differ
across runs.

--symmetrical (or -m)

Ensures the puzzle created will be symmetrical. Most Sudoku puzzles are
symmetrical so it's a good idea usually to use this option.

--solve (or -s) <puzzle>

Solves a puzzle. Argument is string of 81 digits from 0 to 9. The solving
algorithm is surprisingly fast.

--puzzle <puzzle>

Changes the default puzzle for generating from (see the next option). The
default puzzle is a blank board). Argument is string of 81 digits from 0 to 9.

--generate (or -g) <n>

Generates *n* puzzles from the default puzzle. *n* is an integer. This is just a
fun way to see how fast new complete board can be generated. Perhaps it's a way
to benchmark computers?

--depth (or -d) <integer>

Sets the maximum recursive depth to search when solving or creating. Quite a
technical parameter, best ignored in most cases.

--random-seed (or -r) <integer>

Sets the random seed (which otherwise is set by the time).

--verbose (or -v) <0 or 1>

Print out less (0) or more (1). By default, output is verbose.

--test (or -t)

Runs a test suite.

--help (or -h)
Prints this message.

Examples:

- Create a simple puzzle:

        ./sudoku -c 0

- Creates a hard symmetrical puzzle (Warning: This can take anything from a
  second to several hours, depending on chance.)

        ./sudoku -m -c 2

- Solve  puzzle

        ./sudoku -s 300985700008000020000400008000630400005821900009047000600004000010000200002106009

If you wish to use this program's output as input to another program, you may
want to turn off verbosity. E.g.

    ./sudoku -v 0 -c 0

Note that the order of the command line options is important. This wouldn't turn
off verbosity:

    ./sudoku -c 0 -v 0

## Implementation

This is roughly the solving algorithm:

```

search_solution
do
   For each empty cell, fill in the possible numbers for each row, column and square
while any of the cells have changed

while we haven't completed searching the search space or the puzzle is invalid
  Set a cell to a single untried filled value and execute search_solution

```

This is a very simplified version of the creating algorithm:

```
do
  Initialize an empty board
  Choose 16 cells randomly and select a random legal value for each of them.
  do
    Select an empty cell randomly and give it a random selected legal value.
    Check if there is a unique solution.
  while there isn't a unique solution or the puzzle is still valid
while there isn't a unique solution
```

Of course, the source code itself is the definitive description of the
algorithm.
