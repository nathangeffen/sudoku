/*
  Solver: Creates or solves Sudoku problems

  Copyright 2019 Nathan Geffen (See LICENSE)
*/

#ifndef SOLVER_H
#define SOLVER_H

#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
   Sudoku consists of rows, columns and squares. In this code we refer to each
   one of those as a block.  For standard Sudoku each block is 3x3. So we set
   the MINI_BLOCK_SIZE to 3 (known as the rank in Sudoku/math jargon).  Then we
   preset the rows, squares and columns to the indices of 9x9 board Sudoku.

   Nice to do: Code this more flexible to support Sudoku with rank = 4, 5 or
   even 6. (But for 6 we need to change all uint32_t to uint64t.)
*/

#define MINI_BLOCK_SIZE 3
#define BLOCK_SIZE (MINI_BLOCK_SIZE * MINI_BLOCK_SIZE)
#define BOARD_SIZE (BLOCK_SIZE * BLOCK_SIZE)
#define WORD_SIZE 32
#define MAX_SOLUTIONS 2
#define CREATING_MAX_DEPTH 12
#define SOLVING_MAX_DEPTH 100000
#define BITS (BOARD_SIZE / WORD_SIZE + 1)
#define OPTIONAL 0
#define ESSENTIAL 1


static const size_t rows[BLOCK_SIZE][BLOCK_SIZE] = {
    {0,   1,  2,  3,  4,  5,  6,  7,  8 },
    {9,  10, 11, 12, 13, 14, 15, 16, 17 },
    {18, 19, 20, 21, 22, 23, 24, 25, 26 },
    {27, 28, 29, 30, 31, 32, 33, 34, 35 },
    {36, 37, 38, 39, 40, 41, 42, 43, 44 },
    {45, 46, 47, 48, 49, 50, 51, 52, 53 },
    {54, 55, 56, 57, 58, 59, 60, 61, 62 },
    {63, 64, 65, 66, 67, 68, 69, 70, 71 },
    {72, 73, 74, 75, 76, 77, 78, 79, 80 }
};

static const size_t squares[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 1, 2,
     9, 10, 11,
     18, 19, 20},
    {3, 4, 5,
     12, 13, 14,
     21, 22, 23},
    {6, 7, 8,
     15, 16, 17,
     24, 25, 26},
    {27, 28, 29,
     36, 37, 38,
     45, 46, 47},
    {30, 31, 32,
     39, 40, 41,
     48, 49, 50},
    {33, 34, 35,
     42, 43, 44,
     51, 52, 53},
    {54, 55, 56,
     63, 64, 65,
     72, 73, 74},
    {57, 58, 59,
     66, 67, 68,
     75, 76, 77},
    {60, 61, 62,
     69, 70, 71,
     78, 79, 80}
};

static const size_t cols[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 9, 18, 27, 36, 45, 54, 63, 72},
    {1, 10, 19, 28, 37, 46, 55, 64, 73},
    {2, 11, 20, 29, 38, 47, 56, 65, 74},
    {3, 12, 21, 30, 39, 48, 57, 66, 75},
    {4, 13, 22, 31, 40, 49, 58, 67, 76},
    {5, 14, 23, 32, 41, 50, 59, 68, 77},
    {6, 15, 24, 33, 42, 51, 60, 69, 78},
    {7, 16, 25, 34, 43, 52, 61, 70, 79},
    {8, 17, 26, 35, 44, 53, 62, 71, 80}
};

static const size_t lookup[BOARD_SIZE][MINI_BLOCK_SIZE] = {
    {0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {0, 1, 3}, {0, 1, 4}, {0, 1, 5},
    {0, 2, 6}, {0, 2, 7}, {0, 2, 8}, {1, 0, 0}, {1, 0, 1}, {1, 0, 2},
    {1, 1, 3}, {1, 1, 4}, {1, 1, 5}, {1, 2, 6}, {1, 2, 7}, {1, 2, 8},
    {2, 0, 0}, {2, 0, 1}, {2, 0, 2}, {2, 1, 3}, {2, 1, 4}, {2, 1, 5},
    {2, 2, 6}, {2, 2, 7}, {2, 2, 8}, {3, 3, 0}, {3, 3, 1}, {3, 3, 2},
    {3, 4, 3}, {3, 4, 4}, {3, 4, 5}, {3, 5, 6}, {3, 5, 7}, {3, 5, 8},
    {4, 3, 0}, {4, 3, 1}, {4, 3, 2}, {4, 4, 3}, {4, 4, 4}, {4, 4, 5},
    {4, 5, 6}, {4, 5, 7}, {4, 5, 8}, {5, 3, 0}, {5, 3, 1}, {5, 3, 2},
    {5, 4, 3}, {5, 4, 4}, {5, 4, 5}, {5, 5, 6}, {5, 5, 7}, {5, 5, 8},
    {6, 6, 0}, {6, 6, 1}, {6, 6, 2}, {6, 7, 3}, {6, 7, 4}, {6, 7, 5},
    {6, 8, 6}, {6, 8, 7}, {6, 8, 8}, {7, 6, 0}, {7, 6, 1}, {7, 6, 2},
    {7, 7, 3}, {7, 7, 4}, {7, 7, 5}, {7, 8, 6}, {7, 8, 7}, {7, 8, 8},
    {8, 6, 0}, {8, 6, 1}, {8, 6, 2}, {8, 7, 3}, {8, 7, 4}, {8, 7, 5},
    {8, 8, 6}, {8, 8, 7}, {8, 8, 8}
};

/*
  In standard Sudoku these are the possible values of a cell of a completed
  board.
*/

static const uint32_t masks[BLOCK_SIZE] = {
    0b00000000000000000000000000000001,
    0b00000000000000000000000000000010,
    0b00000000000000000000000000000100,
    0b00000000000000000000000000001000,
    0b00000000000000000000000000010000,
    0b00000000000000000000000000100000,
    0b00000000000000000000000001000000,
    0b00000000000000000000000010000000,
    0b00000000000000000000000100000000
};


/* This stores the board */
typedef uint32_t grid_t[BOARD_SIZE];

/* The main data structure */
struct board_s {
    grid_t grid; // The board
    grid_t solutions[MAX_SOLUTIONS]; // Maximum number of solutions (2 default)
    bool complete; // Whether the Sudoku board is complete
    bool valid; // Whether it's valid
    bool too_difficult; // Whether our solver cannot solve it
    int current_index; // Used by the search_solution algorithm to try ab option
    uint32_t current_mask; // Saves the mask so it can be restored
    bool bitboard; // Whether this has been converted to human useable numbers
    int depth; // How deep the solving algorithm had to go.
    int iterations; // Maximum iterations used by the solving algorithm
};


/*
  This is used by the less efficient simple puzzle making algorithm.  It's got a
  second use: Run it many times and then average (or max?) the choices element
  to try to calculate the number of possible completed Sudoku puzzles.
*/
struct board_choices_s {
    struct board_s board, used;
    uint64_t choices; // Used to determine number of possible Sudoku positions
};


/*
  Used solely for the test cases.
*/

struct puzzle_s {
    const char *description;
    const grid_t grid;
    int expected_solutions;
};

/* Test puzzles. */

static struct puzzle_s puzzles[] = {
    {
        "Easy",
        {
            6,0,0, 0,0,0, 1,5,0,
            0,0,0, 0,2,0, 0,0,0,
            7,3,0, 8,0,5, 9,4,0,

            0,4,2, 1,0,0, 0,9,0,
            0,7,0, 9,8,6, 0,2,0,
            0,9,0, 0,0,4, 3,1,0,

            0,1,5, 7,0,2, 0,6,9,
            0,0,0, 0,9,0, 0,0,0,
            0,8,4, 0,0,0, 0,0,5
        }, 1
    },
    {
        "Hard",
         {
             3,0,0, 9,0,0, 0,0,0,
             1,0,5, 0,0,0, 7,0,0,
             0,9,0, 0,0,0, 0,6,1,

             0,8,0, 0,1,0, 9,0,0,
             0,0,7, 0,2,0, 0,0,0,
             0,0,0, 0,0,0, 3,4,0,

             0,0,0, 0,0,0, 0,0,6,
             0,0,0, 0,7,8, 0,9,0,
             0,0,0, 3,0,4, 8,7,0
         }, 1
    },

    {
        "Very hard",
         {
             0,0,0, 0,4,0, 3,0,0,
             0,4,0, 9,7,5, 8,0,0,
             8,0,0, 0,0,0, 0,0,4,

             0,7,0, 0,2,0, 4,0,0,
             0,0,0, 6,0,1, 0,0,0,
             0,0,5, 0,9,0, 0,2,0,

             6,0,0, 0,0,0, 0,0,9,
             0,0,9, 3,6,2, 0,4,0,
             0,0,7, 0,8,0, 0,0,0
         }, 1
    },

    {
        "Extremely hard",
        {
            8,0,0, 0,0,0, 0,0,0,
            0,0,3, 6,0,0, 0,0,0,
            0,7,0, 0,9,0, 2,0,0,

            0,5,0, 0,0,7, 0,0,0,
            0,0,0, 0,4,5, 7,0,0,
            0,0,0, 1,0,0, 0,3,0,

            0,0,1, 0,0,0, 0,6,8,
            0,0,8, 5,0,0, 0,1,0,
            0,9,0, 0,0,0, 4,0,0
        }, 1
    },

    {
        "Broken",
         {
             8,0,0, 0,0,0, 0,0,0,
             0,0,3, 6,0,0, 0,0,0,
             0,7,0, 0,9,0, 2,6,0, // The 6 is wrong

             0,5,0, 0,0,7, 0,0,0,
             0,0,0, 0,4,5, 7,0,0,
             0,0,0, 1,0,0, 0,3,0,

             0,0,1, 0,0,0, 0,6,8,
             0,0,8, 5,0,0, 0,1,0,
             0,9,0, 0,0,0, 4,0,0
         }, 0
    },
    {
        "Complicated broken",
         {
             8,0,0, 0,0,0, 0,0,0,
             0,0,3, 6,0,0, 0,0,0,
             0,7,0, 0,9,0, 2,4,0, // The 4 is wrong

             0,5,0, 0,0,7, 0,0,0,
             0,0,0, 0,4,5, 7,0,0,
             0,0,0, 1,0,0, 0,3,0,

             0,0,1, 0,0,0, 0,6,8,
             0,0,8, 5,0,0, 0,1,0,
             0,9,0, 0,0,0, 4,0,0
         }, 0
    },
    {
        "Multiple solutions",
         {
             8,0,0, 0,0,0, 0,0,0,
             0,0,3, 6,0,0, 0,0,0,
             0,7,0, 0,9,0, 0,0,0, // The 2 is missing

             0,5,0, 0,0,7, 0,0,0,
             0,0,0, 0,4,5, 7,0,0,
             0,0,0, 1,0,0, 0,3,0,

             0,0,1, 0,0,0, 0,6,8,
             0,0,8, 5,0,0, 0,1,0,
             0,9,0, 0,0,0, 4,0,0
         }, MAX_SOLUTIONS
    },
    {
        "Too few specified",
         {
             1,2,3, 4,5,6, 7,8,9,
             5,6,7, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,

             0,0,0, 0,0,1, 0,0,0,
             0,0,0, 0,0,2, 0,0,0,
             0,0,0, 0,0,3, 0,0,0,

             0,0,0, 0,0,0, 8,0,0,
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0
         }, MAX_SOLUTIONS
    },
    {
        "17 entries with multiple solutions",
         {
             1,2,3, 4,5,6, 7,8,9,
             5,6,7, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,

             0,0,0, 0,0,1, 0,0,0,
             0,0,0, 0,0,2, 0,0,0,
             0,0,0, 0,0,3, 0,0,0,

             0,0,0, 0,0,0, 8,0,0,
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 9,0,0
         }, MAX_SOLUTIONS
    },
    {
        "Empty grid",
         {
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,

             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,

             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0,
             0,0,0, 0,0,0, 0,0,0
         }, MAX_SOLUTIONS
    }
};

/* Command line option data structures */

const struct option long_options[] = {
    {"create",       required_argument, 0,  'c' },
    {"symmetrical",  no_argument,       0,  'm' },
    {"solve",        required_argument, 0,  's' },
    {"puzzle",       required_argument, 0,  'p' },
    {"generate",     required_argument, 0,  'g' },
    {"easy",         required_argument, 0,  'e' },
    {"depth",        required_argument, 0,  'd' },
    {"random-seed",  required_argument, 0,  'r' },
    {"verbose",      required_argument, 0,  'v' },
    {"test",         no_argument,       0,  't' },
    {"help",         no_argument,       0,  'h' },
    {0,              0,                 0,   0  }
};

const char *options = "c:ms:p:g:e:d:r:v:th";
const char *arguments[] = {
    "hardness",
    "",
    "puzzle",
    "puzzle",
    "integer",
    "integer",
    "integer",
    "integer",
    "0 or 1",
    "",
    "",
    ""
};

const char *descriptions[] = {
    "Creates a puzzle. Argument of 0 is easiest. 2 or higher is very hard.",
    "Ensures the puzzle created will be symmetrical",
    "Solves a puzzle. Argument is string of 81 digits from 0 to 9.",
    "Changes the default puzzle for generating from (default is blank board).",
    "Generates n puzzles from the default puzzle.",
    "Make easier puzzle (specify maximum filled in cells).",
    "Sets the maximum recursive depth to search.",
    "Sets the random seed (which otherwise is set by the time).",
    "Print out less (0) or more (1). By default, output is verbose.",
    "Runs a test suite.",
    "Prints this message.",
    ""
};

static int verbose = 1;

#endif
