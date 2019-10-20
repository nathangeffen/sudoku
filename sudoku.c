/*
  Solver: Creates or solves Sudoku problems

  Copyright 2019 Nathan Geffen (See LICENSE)
*/

#include "sudoku.h"

uint32_t
set_only_bit(uint32_t n)
{
    return 1 << n;
}

uint32_t
get_bit_index(uint32_t n)
{
    uint32_t result = 0;
    while (n >>= 1) result++;
    return result;
}

uint32_t
count_bits(uint32_t n)
{
    uint32_t count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

void
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

struct board_s
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


uint32_t
get_bit(uint32_t num, uint32_t n)
{
    return (num >> n) & 1;
}

void
get_all_set_bits_as_vals(uint32_t num, uint32_t vals[BLOCK_SIZE])
{
    size_t j = 0;
    for (size_t i = 0; i < BLOCK_SIZE; i++) vals[i] = 0;
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        if (get_bit(num, i)) {
            vals[j] = i + 1;
            j++;
        }
    }
}

void
print_set_bits(uint32_t num)
{
    uint32_t vals[BLOCK_SIZE];
    get_all_set_bits_as_vals(num, vals);
    if (vals[0] == 0) {
        putchar('_');
    } else {
        for (size_t i = 0; i < BLOCK_SIZE && vals[i]; i++) {
            if (i > 0) putchar(',');
            printf("%u", vals[i]);
        }
    }
}

void
print_bitboard(const struct board_s *bitboard)
{
    for (size_t i = 0; i < (BLOCK_SIZE * BLOCK_SIZE); i++) {
        print_set_bits(bitboard->grid[i]);
        if ( (i + 1) == BOARD_SIZE)
            putchar('\n');
        else if ( (i + 1) % (BLOCK_SIZE * MINI_BLOCK_SIZE) == 0)
            printf("\n\n");
        else if ( (i + 1) % BLOCK_SIZE == 0)
            printf("\n");
        else if ( (i + 1) % MINI_BLOCK_SIZE == 0)
            printf("\t\t");
        else
            putchar('\t');
    }
}


void
print_binary(uint32_t n)
{
    for(int i = 0; i < 9; i++) {
        if (n & 1)
            printf("1");
        else
            printf("0");
        n >>= 1;
    }
}

struct board_s
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
                k = blocks[lk_up[1]][j];
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

struct board_s
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

void
print_puzzle(const grid_t grid)
{
    for (size_t i = 0; i < (BLOCK_SIZE * BLOCK_SIZE); i++) {
        if (grid[i] == 0)
            putchar('_');
        else
            printf("%u", get_bit_index(grid[i]) + 1);
        if ( (i + 1) == BOARD_SIZE)
            putchar('\n');
        else if ( (i + 1) % (BLOCK_SIZE * MINI_BLOCK_SIZE) == 0)
            printf("\n\n");
        else if ( (i + 1) % BLOCK_SIZE == 0)
            printf("\n");
        else if ( (i + 1) % MINI_BLOCK_SIZE == 0)
            printf("  ");
        else
            putchar(' ');
    }
}

bool
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

void
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

void
check_bitboard(struct board_s *bitboard)
{
    bitboard->valid = true;
    bitboard->complete = check_bitboard_indices(bitboard, cols);
    bitboard->complete = check_bitboard_indices(bitboard, blocks) &&
        bitboard->complete;
    bitboard->complete = check_bitboard_indices(bitboard, rows) &&
        bitboard->complete;
    if (bitboard->complete && bitboard->valid)
        save_solution(bitboard);
}

void
check_bitboard_comprehensive(struct board_s *bitboard)
{
    *bitboard = fill_possibles(bitboard);
    check_bitboard(bitboard);
}

void
get_next_index(struct board_s *bitboard)
{
    do {
        ++bitboard->current_index;
        bitboard->current_mask = bitboard->grid[bitboard->current_index];
    } while (count_bits(bitboard->current_mask) < 2 &&
             bitboard->current_index < BOARD_SIZE);
}

void
fill(struct board_s *bitboard)
{
    *bitboard = fill_possibles(bitboard);
    *bitboard = fill_exclusions(bitboard, rows);
    *bitboard = fill_possibles(bitboard);
    *bitboard = fill_exclusions(bitboard, blocks);
    *bitboard = fill_possibles(bitboard);
    *bitboard = fill_exclusions(bitboard, cols);
    *bitboard = fill_possibles(bitboard);
}

/*
 * This is the recursive algorithm that searches for a solution to the puzzle.
 * It tries a couple of simple techniques to set the possible values of the
 * missing cells (see the fill function and its subsidiaries) and then
 * if it can't make progress, does a recursive depth first search
 * for a solution.
 */
struct board_s
search_solution(struct board_s bitboard, int depth, int max_depth)
{
    struct board_s prev, new_board;
    int iter = 0;

    if (depth > bitboard.depth)
        bitboard.depth = depth;
    if (bitboard.depth > max_depth) {
        bitboard.too_difficult = true;
        return bitboard;
    }
    do {
        prev = bitboard;
        fill(&bitboard);
        ++iter;
    } while (memcmp(prev.grid,bitboard.grid,BOARD_SIZE*sizeof(uint32_t)) != 0);

    if (iter > bitboard.iterations)
        bitboard.iterations = iter;

    check_bitboard(&bitboard);
    if ( (bitboard.complete && bitboard.valid) ||
         bitboard.current_index == BOARD_SIZE)
        return bitboard;
    else if (bitboard.valid == false)
        return bitboard;

    get_next_index(&bitboard);

    for(size_t i = 0; i < BLOCK_SIZE; i++) {
        if (masks[i] & bitboard.grid[bitboard.current_index]) {
            new_board = bitboard;
            new_board.grid[bitboard.current_index] = masks[i];
            new_board = search_solution(new_board, depth + 1, max_depth);
            for (size_t j = 0; j < MAX_SOLUTIONS; j++)
                if (new_board.solutions[j][0] && !bitboard.solutions[j][0])
                    memcpy(bitboard.solutions[j], new_board.solutions[j],
                           sizeof(new_board.solutions[j]));
                else
                    break;
            if (new_board.depth > bitboard.depth)
                bitboard.depth = new_board.depth;
            if (new_board.solutions[MAX_SOLUTIONS - 1][0])
                return new_board;
        }
    }
    return bitboard;
}

void
solve(struct board_s *bitboard, int max_depth)
{
    check_bitboard_comprehensive(bitboard);

    if (bitboard->valid && bitboard->complete == false)
        *bitboard = search_solution(*bitboard, 0, max_depth);
}

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

void print_result(const struct board_s *board)
{
    int n = num_solutions(board);
    if (board->too_difficult) {
        printf("Puzzle was too hard to solve.\n");
    } else if (n == 1) {
        printf("Unique solution with depth %d\n", board->depth);
        print_puzzle(board->solutions[0]);
    } else if (n > 1) {
        printf("Multiple solutions\n");
        for (size_t j = 0; j < MAX_SOLUTIONS; j++) {
            printf("Solution %zu\n", j);
            print_puzzle(board->solutions[j]);
            printf("\n");
        }
    } else {
        printf("Invalid puzzle\n");
    }
}

void
output_solution(grid_t grid, int max_depth)
{
    struct board_s bitboard;
    bitboard = convert_to_bitboard(grid);
    print_puzzle(bitboard.grid);
    solve(&bitboard, max_depth);

    print_result(&bitboard);
}


void
shuffle(uint32_t arr[],
        size_t n)
{
    for (size_t i = n - 1; i > 0; i--) {
        size_t r = rand() % (i + 1);
        uint32_t t = arr[r];
        arr[r] = arr[i];
        arr[i] = t;
    }
}

void
fill_0_to_n(uint32_t arr[],
            size_t n)
{
    for (size_t i = 0; i < n; i++)
        arr[i] = i;
}

void fill_and_shuffle(uint32_t arr[], size_t n)
{
    fill_0_to_n(arr, n);
    shuffle(arr, n);
}

void
set_random_bit(uint32_t *n)
{
    int i = 0, j = rand() % count_bits(*n) + 1, count = 0;

    for (;count < j; i++)
        if (masks[i] & *n)
            ++count;

    *n = masks[i - 1];
}

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
        printf("Trying ... %u\n", c);

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
            solve(&test_board, max_depth);
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

void
output_puzzle(int min_depth, bool symmetry, int max_depth)
{
    struct board_s board;
    int count = 0;

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
    print_puzzle(board.grid);

    for (size_t i = 0; i < BOARD_SIZE; i++)
        if (count_bits(board.grid[i]) == 1)
            count++;
    printf("Count %u.\n", count);

    print_result(&board);
    for (size_t i = 0; i < BOARD_SIZE; i++)
        if (board.grid[i])
            printf("%u", get_bit_index(board.grid[i]) + 1);
        else
            putchar('0');
    putchar('\n');
}


void
print_help(const char *prog)
{
    printf("%s: A Sudoku puzzle creater and solver\n", prog);
    printf("Options are: \n\n");
    for (int i = 0; i < sizeof(long_options)/sizeof(struct option) - 1; i++) {
        printf("--%s (or -%c) ", long_options[i].name, long_options[i].val);
        if (long_options[i].has_arg == required_argument)
            printf("<%s>", arguments[i]);
        putchar('\n');
        printf("%s\n\n", descriptions[i]);
    }
    printf("Examples:\n");
    printf("%s -c 0\n (Creates a simple puzzle)\n", prog);
    printf("%s -m -c 2\n (Creates a hard symmetrical puzzle)\n", prog);
    printf("%s -s 3009857000080000200004000080006304000058219000090470006"
           "00004000010000200002106009\n"
           "(Solves the puzzle)\n", prog);
}


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

void tests()
{
    int successes = 0, failures = 0;
    size_t n = sizeof(puzzles) / sizeof(struct puzzle_s);

    // Test solver
    for (size_t i = 0; i < n; i++) {
        struct board_s solution;

        printf("Puzzle %zu - %s - before\n", i, puzzles[i].description);
        solution = convert_to_bitboard(puzzles[i].grid);
        print_puzzle(solution.grid);
        solve(&solution, SOLVING_MAX_DEPTH);
        printf("Puzzle %zu - %s - after (max depth: %d, max iterations: %d)\n",
               i, puzzles[i].description, solution.depth,
               solution.iterations);
        int n = num_solutions(&solution);
        if (n == puzzles[i].expected_solutions)
            ++successes;
        else
            ++failures;
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
    print_puzzle(solution.grid);
    print_result(&solution);

    printf("Successes: %d. Failures: %d.\n", successes, failures);
}

int
main(int argc, char *argv[])
{
    int c, i, option_index, symmetry = 0, max_depth = -1;

    srand(time(NULL));

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
            break;
        case 's':
            process_arg_for_solving(optarg, max_depth);
            break;
        case 't':
            tests();
            break;
        case 'h':
            print_help(argv[0]);
            break;
        default:
            print_help(argv[0]);
            break;
        };
    }
    return 0;
}
