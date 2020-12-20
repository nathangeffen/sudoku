/*
  Solver: Creates or solves Sudoku problems

  Copyright 2019 Nathan Geffen (See LICENSE)
*/

#include "sudoku.h"

static char *default_puzzle =
        "000000000000000000000000000000000000000000000000000000000000000000000000000000000";

/* For calling thread safe drand */
static _Thread_local struct drand48_data rng_buf;

/*
   Calls vprintf if verbose is set to true or priority is ESSENTIAL.
*/

static void printf_c(int priority,
              const char *format,
              ...)
{
    if (priority == ESSENTIAL || verbose) {
            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);
    }
}


//////////// Bit functions

/*
   Returns an unsigned integer with only the nth bit set. Zero-based, so
   if n == 0, it returns 1. If n == 1 it returns 2 etc.
*/

static uint32_t
set_only_bit(uint32_t n)
{
    return 1 << n;
}

/*
  Returns the rightmost set bit (but used in the code on integers with just one
  bit set usually). Zero-based. So will return 0 if the first bit is set but
  also if no bits set.
*/

static uint32_t
get_bit_index(uint32_t n)
{
    uint32_t result = 0;
    while (n >>= 1) result++;
    return result;
}

/*
  Counts the number of bits set in an integer.
*/

static uint32_t
count_bits(uint32_t n)
{
    uint32_t count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

/*
  Get's the index in val of the nth bit equal to 1.

  Completely zero-based. i.e.  if val == 0b1 and n == 0, then 0 is returned.
  If val == 0b10 and n == 0 then 1 is returned. if val == 0b1001 and n == 1
  then 3 is returned. If there isn't an nth set bit, then -1 is returned.
 */

int
get_nth_set_bit(uint64_t val, int n)
{
    int i = 0;
    while (val) {
        if (val & 1) --n;
        if (n < 0) return i;
        val >>= 1;
        i++;
    }
    return -1;
}


/*
  Returns the set bits as human readable digits in a string.
 */

char *get_set_bits(uint64_t n) {
    static const char* alphabet = "123456789";
    static _Thread_local char s[BLOCK_SIZE + 1];
    memset(s, 0, sizeof(s));
    int j = 0;
    for (int i = 0; i < BLOCK_SIZE; i++)
        if (n & masks[i]) s[j++] = alphabet[i];
    s[j] = 0;
    return s;
}


/*
   Prints the Sudoku grid as a string. Eg.
   123456789456789123789123456214365897365897241897214365531642978648971532972538614
*/

static void
print_grid_as_str(const grid_t grid) {
    for (size_t i = 0; i < BOARD_SIZE; i++)
        if (grid[i])
            printf("%u", get_bit_index(grid[i]) + 1);
        else
            printf("0");
    printf("\n");
}

/*
  Initializes the board.
 */

static void
init_board(struct board_s *board)
{
    board->current_index = -1;
    board->current_mask = 0;
    board->complete = false;
    board->valid = true;
    board->depth = 0;
    board->too_difficult = false;
    board->iterations = 0;
    memset(board->solutions, 0, sizeof(board->solutions));
}

/*
  Converts a board with human numbers (i.e. 1-9 in standard Sudoku) to
  a bitboard where the nth bit is set to the human interface value. So
  3 will actually be 2^3 or 8.
*/

static struct board_s
convert_to_bitboard(const grid_t puzzle)
{
    struct board_s bitboard;
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        if (puzzle[i])
            bitboard.grid[i] = set_only_bit(puzzle[i] - 1);
        else
            bitboard.grid[i] = 0;
    }
    init_board(&bitboard);

    return bitboard;
}

///////////// Print functions

/*
  Prints the board in human readable form.
  This is the function to use for normal human readable view of a Sudoku puzzle.
*/

static void
print_puzzle(const grid_t grid)
{
    for (size_t i = 0; i < (BLOCK_SIZE * BLOCK_SIZE); i++) {
        if (grid[i] == 0)
            printf_c(ESSENTIAL, "_");
        else
            printf_c(ESSENTIAL, "%u", get_bit_index(grid[i]) + 1);
        if ( (i + 1) == BOARD_SIZE)
            printf_c(ESSENTIAL, "\n");
        else if ( (i + 1) % (BLOCK_SIZE * MINI_BLOCK_SIZE) == 0)
            printf_c(ESSENTIAL, "\n\n");
        else if ( (i + 1) % BLOCK_SIZE == 0)
            printf_c(ESSENTIAL, "\n");
        else if ( (i + 1) % MINI_BLOCK_SIZE == 0)
            printf_c(ESSENTIAL, "  ");
        else
            printf_c(ESSENTIAL, " ");
    }
}

void print_possibles(const struct board_s* board)
{
    for (int i = 0; i < BOARD_SIZE; i++) {
        char *s = get_set_bits(board->grid[i]);
        int j = strlen(s);
        int k = 0;
        if (j == 0) {
            putchar(' ');
        } else if (j == 1) {
            putchar(s[0]);
            k++;
        } else {
            for (; k < j; k++) {
                putchar(s[k]);
            }
        }

        for (; k < BLOCK_SIZE; k++) {
            putchar(' ');
        }
        putchar(' ');

        if ( (i + 1) % 27 == 0) {
            putchar('\n');
            putchar('\n');
        } else if ( (i + 1) % 9 == 0) {
            putchar('\n');
        } else if ( (i + 1) % 3 == 0) {
            putchar(' ');
        }
    }
}



////////////// Solving functions

/*
  Finds all the possible values for a cell and sets the bits corresponding
  to each value.
*/

static struct board_s
fill_possibles(const struct board_s *bitboard)
{
    struct board_s result = *bitboard;
    const size_t *lk_up;
    size_t k;
    uint32_t mask;

    for (size_t i = 0; i < BOARD_SIZE; i++) {
        if (count_bits(result.grid[i]) != 1) {
            mask = 0b11111111111111111111111000000000;
            lk_up = lookup[i];
            for (size_t j = 0; j < BLOCK_SIZE; j++) {
                k = rows[lk_up[0]][j];
                if (k != i && count_bits(result.grid[k]) == 1)
                    mask = mask | result.grid[k];
                k = squares[lk_up[1]][j];
                if (k != i && count_bits(result.grid[k]) == 1)
                    mask = mask | result.grid[k];
                k = cols[lk_up[2]][j];
                if (k != i && count_bits(result.grid[k]) == 1)
                    mask = mask | result.grid[k];
            }
            result.grid[i] = ~mask;
        }
    }
    return result;
}

/*
  Looks for cells which have only one possible value and sets them.
*/

static struct board_s
fill_exclusions(const struct board_s *bitboard,
                const size_t indices[BLOCK_SIZE][BLOCK_SIZE])
{
    struct board_s result = *bitboard;
    uint32_t count;
    size_t l, index;

    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        for (size_t j = 0; j < BLOCK_SIZE; j++) {
            count = 0;
            for (size_t k = 0;  k < BLOCK_SIZE; k++) {
                l = indices[i][k];
                if (result.grid[l] & masks[j]) {
                    count++;
                    index = l;
                }
            }
            if (count == 1)
                result.grid[index] = masks[j];
        }
    }
    return result;
}

/*
  Saves a completed Sudoku puzzle into the solutions array in the bitboard
  struct. Called by the check_bitboard function if it finds that a bitboard
  is complete and valid.
*/

static void
save_solution(struct board_s *bitboard)
{
    for (size_t i = 0; i < MAX_SOLUTIONS; i++) {
        if (bitboard->solutions[i][0] == 0) {
            memcpy(bitboard->solutions[i], bitboard->grid,
                   sizeof(bitboard->grid));
            break;
        } else {
            if (memcmp(bitboard->solutions[i], bitboard->grid,
                       sizeof(bitboard->grid)) == 0)
                break;
        }
    }
}

/*
   Checks if a row, square or column is valid or complete
*/

static bool
check_bitboard_indices(struct board_s *bitboard,
                       const size_t indices[BLOCK_SIZE][BLOCK_SIZE])
{
    bool complete = true;
    for (size_t i = 0; i < BLOCK_SIZE &&
             (bitboard->complete || bitboard->valid); i++) {
        uint32_t mask = 0;
        for (size_t j = 0; j < BLOCK_SIZE; j++) {
            size_t l = indices[i][j];
            if (count_bits(bitboard->grid[l]) == 1) {
                if ( (mask & bitboard->grid[l]) )
                    bitboard->valid = false;
                mask = mask | bitboard->grid[l];
            } else {
                complete = false;
                if ( (bitboard->grid[l] == 0)
                    || (bitboard->grid[l] && (mask ^ bitboard->grid[l]) == 0) )
                    bitboard->valid = false;
            }
        }
    }
    return complete;
}


/*
  Checks a bitboard for completeness or validity. Saves it in the solutions
  array of the bitboard struct if it is complete and valid.
 */

static void
check_bitboard(struct board_s *bitboard)
{
    bitboard->valid = true;
    bitboard->complete = check_bitboard_indices(bitboard, cols);
    bitboard->complete = check_bitboard_indices(bitboard, squares) &&
        bitboard->complete;
    bitboard->complete = check_bitboard_indices(bitboard, rows) &&
        bitboard->complete;
    if (bitboard->complete && bitboard->valid)
        save_solution(bitboard);
}

/*
  Finds the next cell in the bitboard with more than one possible value.
  The search_solution algorithm will then try one of the values in this cell.
*/

static void
get_next_cell(struct board_s *bitboard)
{
    do {
        ++bitboard->current_index;
        bitboard->current_mask = bitboard->grid[bitboard->current_index];
    } while (count_bits(bitboard->current_mask) < 2 &&
             bitboard->current_index < BOARD_SIZE);
}

/*
  Repeatedly tries to fill values in until no progress can be made.
*/

static void
fill(struct board_s *bitboard)
{
    int iter = 0;
    struct board_s prev;
    do {
        prev = *bitboard;
        *bitboard = fill_possibles(bitboard);
        *bitboard = fill_exclusions(bitboard, rows);
        *bitboard = fill_possibles(bitboard);
        *bitboard = fill_exclusions(bitboard, squares);
        *bitboard = fill_possibles(bitboard);
        *bitboard = fill_exclusions(bitboard, cols);
        *bitboard = fill_possibles(bitboard);
        ++iter;
    } while (memcmp(prev.grid,bitboard->grid,BOARD_SIZE*sizeof(uint32_t)) != 0);

    if (iter > bitboard->iterations)
        bitboard->iterations = iter;
}

/*
  Fills as much of the bitboard as possible without needing recursion
  and then checks if it's valid.
 */

static void
check_bitboard_comprehensive(struct board_s *bitboard)
{
    fill(bitboard);
    check_bitboard(bitboard);
}

/*
 * This is the recursive algorithm that searches for a solution to the puzzle.
 * It tries a couple of simple techniques to set the possible values of the
 * missing cells (see the fill function and its subsidiaries) and then
 * if it can't make progress, does a recursive depth first search
 * for a solution.
 */
struct board_s
search_solution(struct board_s bitboard, int depth, int max_depth, int *generate)
{
    struct board_s new_board;

    if (depth > bitboard.depth)
        bitboard.depth = depth;
    if (bitboard.depth > max_depth) {
        bitboard.too_difficult = true;
        return bitboard;
    }

    fill(&bitboard);

    check_bitboard(&bitboard);
    if ( (bitboard.complete && bitboard.valid) ||
         bitboard.current_index == BOARD_SIZE) {
        if (*generate > 0) {
            --*generate;
            printf("%d,", *generate);
            print_grid_as_str(bitboard.grid);
        }
        return bitboard;
    } else if (bitboard.valid == false) {
        return bitboard;
    }

    get_next_cell(&bitboard);

    for(size_t i = 0; i < BLOCK_SIZE; i++) {
        if (masks[i] & bitboard.grid[bitboard.current_index]) {
            new_board = bitboard;
            new_board.grid[bitboard.current_index] = masks[i];
            new_board = search_solution(new_board, depth + 1, max_depth,
                                        generate);
            if (*generate < 0) {
                for (size_t j = 0; j < MAX_SOLUTIONS; j++)
                    if (new_board.solutions[j][0] && !bitboard.solutions[j][0])
                        memcpy(bitboard.solutions[j], new_board.solutions[j],
                               sizeof(new_board.solutions[j]));
                    else
                        break;
            } else {
                if (*generate == 0) return new_board;
            }
            if (new_board.depth > bitboard.depth)
                bitboard.depth = new_board.depth;
            if (new_board.solutions[MAX_SOLUTIONS - 1][0])
                return new_board;
        }
    }
    return bitboard;
}


/*
  Wrapper around the recursive search_solutions algorithm.
 */

void
solve(struct board_s *bitboard, int max_depth, int generate)
{
    check_bitboard_comprehensive(bitboard);

    if (bitboard->valid && bitboard->complete == false)
        *bitboard = search_solution(*bitboard, 0, max_depth, &generate);
}

/*
  Checks how many solutions have been found after solve has been called.
 */

int
num_solutions(const struct board_s *board)
{
    int c = 0;
    for (size_t i = 0; i < MAX_SOLUTIONS; i++)
        if (board->solutions[i][0] > 0)
            c++;
        else
            break;

    return c;
}

/*
  User friendly printout of results of attempt to solve puzzle.
 */

void print_result(const struct board_s *board)
{
    int n = num_solutions(board);
    if (board->too_difficult) {
        printf_c(ESSENTIAL, "Puzzle was too hard to solve.\n");
    } else if (n == 1) {
        printf_c(ESSENTIAL, "Unique solution with depth %d\n",
                 board->depth);
        if (verbose) {
            print_puzzle(board->solutions[0]);
        }
    } else if (n > 1) {
        printf_c(ESSENTIAL, "Multiple solutions\n");
        for (size_t j = 0; j < MAX_SOLUTIONS; j++) {
            printf_c(ESSENTIAL, "Solution %zu\n", j);
            print_puzzle(board->solutions[j]);
            printf_c(ESSENTIAL, "\n");
        }
    } else {
        printf_c(ESSENTIAL, "Invalid puzzle\n");
    }
}

/*
  Prints puzzle information. If verbosity is on, prints count which indicates
  how much of the board is solved, and depth which indicates puzzle difficulty.
*/

void
print_puzzle_info(const struct board_s *board)
{
    int count = 0;
    for (size_t i = 0; i < BOARD_SIZE; i++)
        if (count_bits(board->grid[i]) == 1)
            count++;
    printf_c(ESSENTIAL, "Count: %d\n", count);
    printf_c(ESSENTIAL, "Depth: %d\n", board->depth);

    printf_c(ESSENTIAL, "Puzzle: ", board->depth);
    print_grid_as_str(board->grid);
}

/*
  Prints the solutions to a puzzle.
 */

void
output_solution(grid_t grid, int max_depth)
{
    struct board_s board;
    int n;
    board = convert_to_bitboard(grid);
    if (verbose)
        print_puzzle(board.grid);
    solve(&board, max_depth, -1);

    if (verbose)
        print_result(&board);

    n = num_solutions(&board);
    for (int i = 0; i < n; i++) {
        printf("%d,", i+1);
        print_grid_as_str(board.solutions[i]);
    }
}

/*
  Shuffles an array of unsigned integers.
*/

void
shuffle(uint32_t arr[],
        size_t n)
{
    for (size_t i = n - 1; i > 0; i--) {
        long r;
        lrand48_r(&rng_buf, &r);
        r = r % (i + 1);
        uint32_t t = arr[r];
        arr[r] = arr[i];
        arr[i] = t;
    }
}

/*
  Fills an array from 0 to n-1.
 */

void
fill_0_to_n(uint32_t arr[],
            size_t n)
{
    for (size_t i = 0; i < n; i++)
        arr[i] = i;
}

/*
  Fills an array from 0 to n-1 and then shuffles it.
 */

void fill_and_shuffle(uint32_t arr[], size_t n)
{
    fill_0_to_n(arr, n);
    shuffle(arr, n);
}

/*
  Randomly sets an integer to have one and only one of the first n bits set
  where n is the size of the Sudoku alphabet (9 in the default version).
*/

void
set_random_bit(uint32_t *n)
{
    int i = 0, j = rand() % count_bits(*n) + 1, count = 0;

    for (;count < j; i++)
        if (masks[i] & *n)
            ++count;

    *n = masks[i - 1];
}

/* Creates puzzles that are generally very hard to solve.
   The higher min_depth the harder (and slower to create) it is. But
   even leaving min_depth at 0 generally makes it hard enough. Setting
   max_depth too high may result in a very long time to create some puzzles.
*/

struct board_s
create_puzzle(int min_depth,
              int max_depth,
              bool symmetry)
{
    uint32_t cells[BOARD_SIZE], *cell, *mirror_cell, i, mask, n, c = 0;
    struct board_s board, test_board;

    // Loop until we have a valid puzzle with a unique solution
    // of sufficient depth
    do {
        printf_c(OPTIONAL, "Trying ... %u\n", c);

        // We use two boards. One will be passed to the solving algorithm
        // and will used to test for a solution.
        // The other holds the current position.
        init_board(&board);
        init_board(&test_board);
        n = 0;
        memset(board.grid, 0, sizeof(board.grid));

        // Shuffle an array of indices into the board
        if (symmetry)
            fill_and_shuffle(cells, BOARD_SIZE / 2 + 1);
        else
            fill_and_shuffle(cells, BOARD_SIZE);

        // Randomly, but legally, set 16 cells
        for (i = 0; i < 16 / ((int) (symmetry + 1)); i++) {
            fill(&board);
            if (board.grid[cells[i]] == 0) {
                board.valid = false;
                break;
            }
            set_random_bit(&board.grid[cells[i]]);
            if (symmetry && cells[i] != BOARD_SIZE / 2 + 1) {
                fill(&board);
                if (board.grid[BOARD_SIZE - cells[i] - 1] == 0) {
                    board.valid = false;
                } else {
                    assert(count_bits(board.grid[BOARD_SIZE - cells[i] - 1]));
                    set_random_bit(&board.grid[BOARD_SIZE - cells[i] - 1]);
                }
            }
        }

        if (board.valid == false)
            continue;
        fill(&board);

        // Now loop until a solution is found or the board is invalid
        // On each iteration, randomly select an unfilled cell,
        // set it to a randomly selected legal value, and
        // check for a solution
        do {
            cell = &board.grid[cells[i]];
            mask = *cell;
            if (*cell == 0) {
                board.valid = false;
                break;
            }
            assert(count_bits(*cell));
            set_random_bit(cell);
            memcpy(&test_board, &board, sizeof(board));
            fill(&test_board);
            if (symmetry && cells[i] != BOARD_SIZE / 2 + 1) {
                mirror_cell = &board.grid[BOARD_SIZE - cells[i] - 1];
                if (*mirror_cell == 0) {
                    board.valid = false;
                    break;
                }
                assert(count_bits(*mirror_cell));
                set_random_bit(mirror_cell);
                memcpy(&test_board, &board, sizeof(board));
                fill(&test_board);
            }
            solve(&test_board, max_depth, -1);
            n = num_solutions(&test_board);
            if (n == 0) {
                *cell = mask ^ *cell;
                if (*cell == 0 || test_board.depth > max_depth) {
                    board.valid = false;
                    break;
                }
            } else {
                i++;
            }
        } while (n != 1 && i < (BOARD_SIZE / ((int) symmetry + 1) ) );
        c++;
    } while (test_board.depth < min_depth || test_board.depth > max_depth ||
             n != 1 || board.valid == false);

    // We have a valid solution in test_board and the starting grid in board.
    // So copy the starting grid into test_board and that's what we return.
    memcpy(test_board.grid, board.grid, sizeof(test_board.grid));
    for (i = 0; i < BOARD_SIZE; i++)
        if (count_bits(test_board.grid[i]) > 1)
            test_board.grid[i] = 0;

    return test_board;
}

/*
  This is used to create a random complete Sudoku board from which simpler
  puzzles can be created. It's an order of magnitude slower at solving than the
  search_solution algorithm. But it is useful for calculating the likely number
  of completed Sudoku puzzles.
*/
struct board_choices_s
make_random_complete_board(struct board_s *board)
{
    const size_t stack_size = 1000;
    struct board_choices_s stack[stack_size];
    struct board_s b, c;
    int sp = 0;

    b = *board;

    fill(&b);

    stack[sp].board = b;
    stack[sp].choices = 1;

    init_board(&stack[sp].used);
    memset(&stack[sp].used.grid, 0, sizeof(stack[sp].used.grid));

    ++sp;
    check_bitboard(&stack[sp-1].board);

    while(sp > 0 && sp < stack_size && stack[sp-1].board.complete == false) {
        b = stack[sp-1].board;
        c = stack[sp-1].used;

        fill(&b);

        // Remove used options
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (c.grid[i])
                b.grid[i] = ~c.grid[i] & b.grid[i];
        }

        check_bitboard(&b);
        if (b.valid == false) {
            --sp;
            continue;
        }

        // Find the cell with the fewest options > 1
        int min_index = 0, min_bits = BLOCK_SIZE + 1;
        for (int i = 0; i < BOARD_SIZE; i++) {
            int j = count_bits(b.grid[i]);
            if (j > 1 && j < min_bits) {
                min_index = i;
                min_bits = j;
            }
        }
        long n;
        lrand48_r(&rng_buf, &n);
        n = n % min_bits;
        int nth_bit = get_nth_set_bit(b.grid[min_index], n);
        b.grid[min_index] = masks[nth_bit];

        stack[sp-1].used.grid[min_index] |= masks[nth_bit];
        stack[sp].board = b;
        init_board(&stack[sp].used);
        memset(&stack[sp].used.grid, 0, sizeof(stack[sp].used.grid));
        stack[sp].choices = min_bits;
        ++sp;
    }
    if (sp == 0 || sp == stack_size) {
        fill(&stack[0].board);
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (stack[0].used.grid[i])
                stack[0].board.grid[i] = ~stack[0].used.grid[i] &
                    stack[0].board.grid[i];
        }
        init_board(&stack[0].board);
        stack[0].board.valid = false;
        stack[0].board.complete = false;
        stack[0].choices = 0;
        return stack[0];
    } else {
        uint64_t choices = 1;
        for (int i = 0; i < sp; i++)
            choices *= stack[i].choices;
        stack[sp-1].board.complete = true;
        stack[sp-1].board.valid = true;
        memcpy(stack[sp-1].board.grid, stack[sp-1].board.solutions[0],
               BOARD_SIZE*sizeof(uint32_t));
        stack[sp-1].choices = choices;
        return stack[sp-1];
    }
}

bool
unique_solution(struct board_s board)
{
    solve(&board, SOLVING_MAX_DEPTH, -1);
    if (board.valid && num_solutions(&board) == 1)
        return true;
    else
        return false;
}

/*
  Makes easyish puzzles. At least not usually as hard as the default method.
  The symmetry parameter ensures the puzzle is symmetrical. The min_removals
  parameter is the minimum number of blank squares needed for the puzzle.
  Note that the higher the value of min_removals the slower this will be and at
  some high value it sends this function into an endless loop.
*/

struct board_s
make_easy_puzzle(bool symmetry, int min_removals) {
    struct board_s board, prev;
    int i;
    if (symmetry) min_removals /= 2;
    do {
        i = 0;
        struct board_choices_s bc;
        uint32_t shuffled_indices[BOARD_SIZE];
        int num_cells;

        if (symmetry) {
            num_cells = BOARD_SIZE / 2 + 1;
        } else {
            num_cells = BOARD_SIZE;
        }
        fill_and_shuffle(shuffled_indices, num_cells);

        init_board(&board);
        memset(board.grid, 0, sizeof(board.grid));
        bc = make_random_complete_board(&board);

        memcpy(board.grid, bc.board.solutions[0], BOARD_SIZE * sizeof(uint32_t));
        board.complete = false;

        for (; i < num_cells && (i < min_removals || min_removals == 0); i++) {
            int r = shuffled_indices[i];
            prev = board;
            board.grid[r] = 0;
            if (symmetry && r != 40)
                board.grid[80 - r] = 0;
            if (unique_solution(board) == false)
                break;
        }
    } while (i < min_removals);
    if (min_removals)
        return board;
    else
        return prev;
}

/*
  Wrapper function for creating a new puzzle.
*/

void
output_puzzle(int min_depth, bool symmetry, int max_depth)
{
    struct board_s board;

    if (max_depth - min_depth < 3) {
        fprintf(stderr, "Depth for puzzle too close to max depth.\n");
        exit(EXIT_FAILURE);
    }

    if (max_depth < 0) {
        fprintf(stderr, "Depths must be positive.\n");
        exit(EXIT_FAILURE);
    }

    if (min_depth > CREATING_MAX_DEPTH) {
        fprintf(stderr, "A depth of %d is too ambitious.\n", min_depth);
        exit(EXIT_FAILURE);
    }

    board = create_puzzle(min_depth, max_depth, symmetry);
    if (verbose)
        print_puzzle(board.grid);

    if (verbose)
        print_result(&board);

    print_puzzle_info(&board);
}

/*
  CLI help.
 */

void
print_help(const char *prog)
{
    printf_c(ESSENTIAL, "%s: A Sudoku puzzle creater and solver\n", prog);
    printf_c(ESSENTIAL, "Options are: \n\n");
    for (int i = 0; i < sizeof(long_options)/sizeof(struct option) - 1; i++) {
        printf_c(ESSENTIAL, "--%s (or -%c) ", long_options[i].name, long_options[i].val);
        if (long_options[i].has_arg == required_argument)
            printf_c(ESSENTIAL, "<%s>", arguments[i]);
        putchar('\n');
        printf_c(ESSENTIAL, "%s\n\n", descriptions[i]);
    }
    printf_c(ESSENTIAL, "Examples:\n");
    printf_c(ESSENTIAL, "%s -c 0\n (Creates a simple puzzle)\n", prog);
    printf_c(ESSENTIAL, "%s -m -c 2\n (Creates a hard symmetry puzzle)\n", prog);
    printf_c(ESSENTIAL, "%s -s 3009857000080000200004000080006304000058219000090470006"
           "00004000010000200002106009\n"
           "(Solves the puzzle)\n", prog);
}

/*
  Processes the command line option for solving a puzzle.
 */

void
process_arg_for_solving(char *puzzle_string,
                        int max_depth)
{
    uint32_t *g;
    int l;
    char *c;
    grid_t grid;

    l = strlen(optarg);
    if (l < BOARD_SIZE) {
        fprintf(stderr, "Too few cells specified\n");
        exit(EXIT_FAILURE);
    } else if (l > BOARD_SIZE) {
        fprintf(stderr, "Too many cells specified\n");
        exit(EXIT_FAILURE);
    }
    for (c = puzzle_string, g = grid; *c; c++, g++) {
        if (*c >= '0' && *c <= '9') {
            *g = (uint32_t) (*c - '0');
        } else {
            fprintf(stderr, "Incorrect character used %c\n", *c);
            exit(EXIT_FAILURE);
        }
    }
    output_solution(grid, (max_depth == -1) ? SOLVING_MAX_DEPTH : max_depth);
}

/*
  Allows the user to change the puzzle from which generation (creation of n
  complete Sudoku board) will take place.
*/

void
set_default_puzzle(const char *puzzle_str)
{
    int n = strlen(default_puzzle);
    if (n != BOARD_SIZE) {
        fprintf(stderr, "Incorrect puzzle length %d\n", n);
        exit(1);
    }

    for (const char *c = puzzle_str; *c; c++) {
        if (*c < '0' || *c <= '9') {
            fprintf(stderr, "Incorrect character used %c\n", *c);
            exit(EXIT_FAILURE);
        }
    }

    strcpy(default_puzzle, puzzle_str);
}

/*
  Wrapper function for generating as many complete Sudoku boards as possible.
*/

void
process_arg_for_generating(int num_solutions,
                           int max_depth)
{
    uint32_t *g;
    char *c;
    grid_t grid;

    struct board_s board;

    for (c = default_puzzle, g = grid; *c; c++, g++)
        *g = (uint32_t) (*c - '0');

    board = convert_to_bitboard(grid);
    solve(&board, SOLVING_MAX_DEPTH, num_solutions);
}


/*
  Process the make easy puzzle command line argument. Note the puzzle may not be
  easy at all. But in general should be easier than the default puzzle creation
  algorithm. max_cells specifies the maximum number of cells that may be filled
  in.
*/

void
process_arg_for_easy(int symmetry, int max_cells)
{
    struct board_s board = make_easy_puzzle(symmetry, max_cells);
    print_grid_as_str(board.grid);
}



/*
  Tests that most of the above functions work as expected. Never foolproof of
  course.
 */

void tests()
{
    int successes = 0, failures = 0;
    size_t n = sizeof(puzzles) / sizeof(struct puzzle_s);

    // Test simple puzzle creator
    struct board_s board = make_easy_puzzle(true, 20);
    if (verbose)
        print_grid_as_str(board.grid);
    board = convert_to_bitboard(board.grid);
    solve(&board, SOLVING_MAX_DEPTH, -1);
    if (num_solutions(&board) == 1) {
        ++successes;
    } else {
        printf_c(ESSENTIAL, "Easy puzzle has more than one solution\n");
        print_grid_as_str(board.grid);
        ++failures;
    }

    // Test solver
    for (size_t i = 0; i < n; i++) {
        struct board_s solution;

        printf_c(OPTIONAL, "Puzzle %zu - %s - before\n",
                 i, puzzles[i].description);
        solution = convert_to_bitboard(puzzles[i].grid);
        if (verbose)
            print_puzzle(solution.grid);
        solve(&solution, SOLVING_MAX_DEPTH, -1);
        printf_c(OPTIONAL, "",
                 "Puzzle %zu - %s - after (max depth: %d, max iterations: %d)\n",
               i, puzzles[i].description, solution.depth,
               solution.iterations);
        int n = num_solutions(&solution);
        if (n == puzzles[i].expected_solutions)
            ++successes;
        else
            ++failures;
        if (verbose)
            print_result(&solution);
    }

    // Test creator
    srand(2);
    struct board_s solution = create_puzzle(1, CREATING_MAX_DEPTH, true);
    n = num_solutions(&solution);
    if (n == 1 && solution.valid == true && solution.depth >= 1)
        ++successes;
    else
        ++failures;
    if (verbose) {
        print_puzzle(solution.grid);
        print_result(&solution);
    }
    printf_c(ESSENTIAL, "Successes: %d. Failures: %d.\n",
             successes, failures);

}

int
main(int argc, char *argv[])
{
    int c, i, option_index, symmetry = 0, max_depth = -1;

    srand48_r(time(NULL), &rng_buf);

    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, options,
                        long_options, &option_index);
        if (c == -1)
            break;
        switch(c) {
        case 'c':
            i = atoi(optarg);
            output_puzzle(i, (bool) symmetry,
                          (max_depth == -1) ? CREATING_MAX_DEPTH : max_depth);
            break;
        case 'm':
            symmetry = 1;
            break;
        case 'd':
            max_depth = atoi(optarg);
            break;
        case 'r':
            srand(atoi(optarg));
            srand48_r(atoi(optarg), &rng_buf);
            break;
        case 's':
            process_arg_for_solving(optarg, max_depth);
            break;
        case 'p':
            set_default_puzzle(optarg);
            break;
        case 'g':
            process_arg_for_generating(atoi(optarg), max_depth);
            break;
        case 'e':
            process_arg_for_easy(symmetry, atoi(optarg));
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 't':
            tests();
            break;
        case 'h':
            print_help(argv[0]);
            break;
        default:
            print_help(argv[0]);
            exit(EXIT_FAILURE);
            break;
        };
    }

    return 0;
}
