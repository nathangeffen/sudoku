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

- To creates a puzzle. Argument of 0 is easiest. 2 or higher is very hard.

     --create (or -c) <hardness>

- Ensures the puzzle created will be symmetrical (place before -c)

     --symmetrical (or -m)

- Solve a puzzle. Argument is string of 81 digits from 0 to 9.

     --solve (or -s) <puzzle>

- Set the maximum recursive depth to search (should seldom be used)

     --depth (or -d) <integer>

- Set the random seed (which otherwise is set by the time).

     --random-seed (or -r) <integer>

- Runs a test suite.

     --test (or -t)

- Prints help.

      --help (or -h)

Examples:

- Create a simple puzzle:

    ./sudoku -c 0

- Creates a hard symmetrical puzzle (Warning: This can take anything from a
  second to several hours, depending on chance.)

    ./sudoku -m -c 2

- Solve  puzzle

   ./sudoku -s 300985700008000020000400008000630400005821900009047000600004000010000200002106009

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
