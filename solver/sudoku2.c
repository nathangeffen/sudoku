#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#define MINI_BLOCK_SIZE 3
#define BLOCK_SIZE (MINI_BLOCK_SIZE * MINI_BLOCK_SIZE)
#define BOARD_SIZE (BLOCK_SIZE * BLOCK_SIZE)

#define MIN(a, b)  ( (a) < (b) ) ? (a) : (b)

static const char* alphabet = "0123456789";

static const struct option long_options[] = {
    {"solve",       required_argument, 0,  's' },
    {"generate",    required_argument, 0,  'g' },
    {"very-easy",   no_argument,       0,  'v' },
    {"easy",        no_argument,       0,  'e' },
    {"medium",      no_argument,       0,  'm' },
    {"hard",        no_argument,       0,  'd' },
    {"random-seed", required_argument, 0,  'r' },
    {"test",        required_argument, 0,  't' },
    {"help",        no_argument,       0,  'h' },
    {0,             0,                 0,   0  }
};

static const char *options = "s:g:vemdr:th";

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

static const size_t blocks[BLOCK_SIZE][BLOCK_SIZE] = {
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

static const uint64_t masks[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};

static const uint64_t all_ones = 1|2|4|8|16|32|64|128|256;

struct board_s {
    uint64_t cells[BOARD_SIZE];
    bool solved;
    bool broken;
};


static _Thread_local struct drand48_data rng_buf;

void
print_help(const char *prog)
{
    printf("%s: A Sudoku puzzle creater and solver\n", prog);
    printf("Options are: \n\n");
    for (int i = 0; i < sizeof(long_options)/sizeof(struct option) - 1; i++)
        printf("--%s (or -%c)\n", long_options[i].name, long_options[i].val);
}

char* binary(uint64_t n, int digits)
{
    static _Thread_local char s[65];
    memset(s, 0, sizeof(s));
    char *c = s;
    bool one_started = false;
    uint64_t two_64 = 1L<<63;

    for (int i = 0; i < 64; i++) {
        if (n & two_64) {
            *c++ = '1';
            one_started = true;
        } else if (one_started || (64 - i) - digits <= 0) {
                *c++ = '0';
        }
        n <<= 1;
    }
    return s;
}


int
num_bits(uint64_t n)
{
    uint64_t count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

uint64_t
set_only_bit(unsigned char n)
{
    return masks[n];
}

/* Get the position in an integer of the first set bit */
int
get_set_bit_pos(uint64_t n)
{
    int i = 0;
    for (;n && ++i && (n & 1) == 0;)
        n >>= 1;
    return i;
}

char *get_set_bits(uint64_t n) {
    static _Thread_local char s[BLOCK_SIZE + 1];
    memset(s, 0, sizeof(s));
    int j = 0;
    for (int i = 1; i <= BLOCK_SIZE; i++)
        if (n & masks[i]) s[j++] = alphabet[i];
    s[j] = 0;
    return s;
}

uint64_t
get_block_bits(const struct board_s *board, int index, const size_t block[])
{
    uint64_t result = 0;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        int j = block[i];
        if (j != index && num_bits(board->cells[j]) == 1)
            result = result | board->cells[j];
    }
    return result;
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

int
set_bit_to_zero(int val, int n)
{
    return (1 << n) ^ val;
}

void
init_board(struct board_s *board)
{
    memset(board->cells, 0, BOARD_SIZE * sizeof(uint64_t));
    board->solved = board->broken = false;
}

bool str_contains(const char *str, char c, int n)
{
    for (int i = 0; i < n; i++)
        if (str[i] == c) return true;
    return false;
}

struct board_s
convert_str_board(const char *puzzle) {
    struct board_s board;

    if (strlen(puzzle) != BOARD_SIZE) {
        fprintf(stderr, "Not enough chars in puzzle.\n");
        exit(1);
    }

    init_board(&board);
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (str_contains(alphabet, puzzle[i], BLOCK_SIZE+1) == false) {
            fprintf(stderr, "Invalid char %c in puzzle at position %d.\n",
                   puzzle[i], i);
            exit(1);
        }
        board.cells[i] = set_only_bit(puzzle[i] - alphabet[0]);
    }
    board.solved = board.broken = false;
    return board;
}

void
convert_board_str(const struct board_s *board, char puzzle[]) {
    for (int i = 0; i < BOARD_SIZE; i++)
        puzzle[i] = alphabet[get_set_bit_pos(board->cells[i])];
    puzzle[BOARD_SIZE] = 0;
}

void print_board(const struct board_s* board, bool possibles)
{
    for (int i = 0; i < BOARD_SIZE; i++) {
        char *s = get_set_bits(board->cells[i]);
        int j = strlen(s);
        int k = 0;
        if (j == 0) {
            putchar(' ');
        } else if (j == 1) {
            putchar(s[0]);
            k++;
        } else {
            if (possibles) {
                for (; k < j; k++) {
                    putchar(s[k]);
                }
            } else {
                putchar(' ');
            }
        }
        if (possibles) {
            for (; k < BLOCK_SIZE; k++) {
                putchar(' ');
            }
            putchar(' ');
        }
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

void
copy_board(struct board_s *to, struct board_s *from)
{
    memcpy(to, from, sizeof(*to));
}

bool
is_valid_block(const struct board_s *board, int index, const size_t block[])
{
    for (int i = 0; i < BLOCK_SIZE; i++) {
        int j = block[i];
        if (j == index || num_bits(board->cells[j]) > 1)
            continue;
        if (board->cells[j] == board->cells[index])
            return false;
    }
    return true;
}

bool
is_valid(const struct board_s *board, bool zero_broken)
{
    const size_t *l;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (zero_broken && board->cells[i] == 0) return false;
        if (num_bits(board->cells[i]) != 1) continue;
        l = lookup[i];
        if (is_valid_block(board, i, rows[l[0]]) &&
            is_valid_block(board, i, blocks[l[1]]) &&
            is_valid_block(board, i, cols[l[2]]))
            ;
        else
            return false;
    }
    return true;
}

bool is_broken(const struct board_s *board, bool zero_broken)
{
    return !is_valid(board, zero_broken);
}

bool is_complete(const struct board_s *board)
{
    for (int i = 0; i < BOARD_SIZE; i++)
        if (num_bits(board->cells[i]) != 1)
            return false;
    if (is_valid(board, false))
        return true;
    else
        return false;

}

void
fill_possibles(struct board_s *board)
{
    const size_t *l;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (num_bits(board->cells[i]) != 1) {
            l = lookup[i];
            uint64_t r, b, c, x;
            r = get_block_bits(board, i, rows[l[0]]);
            b = get_block_bits(board, i, blocks[l[1]]);
            c = get_block_bits(board, i, cols[l[2]]);
            x = (~(r | b | c)) & all_ones;
            board->cells[i] = x;
        }
    }
}

void
fill_exclusions(struct board_s *board,
                const size_t block[BLOCK_SIZE][BLOCK_SIZE])
{
    int count, index;

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 1; j < BLOCK_SIZE+1; j++) {
            count = 0;
            for (int k = 0;  k < BLOCK_SIZE; k++) {
                int l = block[i][k];
                if (board->cells[l] & masks[j]) {
                    count++;
                    index = l;
                }
            }
            if (count == 1)
                board->cells[index] = masks[j];
        }
    }
}

void
fill_possibles_continuously(struct board_s *board)
{
    struct board_s prev;
    do {
        prev = *board;
        fill_possibles(board);
        fill_exclusions(board, rows);
        fill_possibles(board);
        fill_exclusions(board, blocks);
        fill_possibles(board);
        fill_exclusions(board, cols);
        fill_possibles(board);
    } while (memcmp(board->cells, prev.cells, sizeof(board->cells)));
}

struct board_choices_s {
    struct board_s board, used;
    uint64_t choices;
};

struct board_choices_s
fill_cells(struct board_s *board, bool rnd)
{
    const size_t stack_size = 1000;
    struct board_choices_s stack[stack_size];
    struct board_s b, c;
    int sp = 0;

    b = *board;
    fill_possibles_continuously(&b);

    stack[sp].board = b;
    stack[sp].choices = 1;
    init_board(&stack[sp].used);
    ++sp;

    while(sp > 0 && sp < stack_size &&
          is_complete(&stack[sp-1].board) == false) {
        b = stack[sp-1].board;
        c = stack[sp-1].used;
        fill_possibles_continuously(&b);

        // Remove used options
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (c.cells[i])
                b.cells[i] = ~c.cells[i] & b.cells[i];
        }

        if (is_broken(&b, true)) {
            --sp;
            continue;
        }

        // Find the cell with the fewest options > 1
        int min_index = 0, min_bits = BLOCK_SIZE + 1;
        for (int i = 0; i < BOARD_SIZE; i++) {
            int j = num_bits(b.cells[i]);
            if (j > 1 && j < min_bits) {
                min_index = i;
                min_bits = j;
            }
        }
        long n;
        if (rnd) {
            lrand48_r(&rng_buf, &n);
            n = n % min_bits;
        } else {
            n = 0;
        }
        int nth_bit = get_nth_set_bit(b.cells[min_index], n) + 1;
        b.cells[min_index] = masks[nth_bit];
        stack[sp-1].used.cells[min_index] |= masks[nth_bit];
        stack[sp].board = b;
        init_board(&stack[sp].used);
        stack[sp].choices = min_bits;
        ++sp;
    }
    if (sp == 0 || sp == stack_size) {
        fill_possibles(&stack[0].board);
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (stack[0].used.cells[i])
                stack[0].board.cells[i] = ~stack[0].used.cells[i] & stack[0].board.cells[i];
        }
        init_board(&stack[0].board);
        stack[0].board.broken = true;
        stack[0].board.solved = false;
        stack[0].choices = 0;
        return stack[0];
    } else {
        uint64_t choices = 1;
        for (int i = 0; i < sp; i++)
            choices *= stack[i].choices;
        stack[sp-1].board.solved = true;
        stack[sp-1].choices = choices;
        return stack[sp-1];
    }
}

int
solve(const struct board_s *board, struct board_s solutions[],
      int max_solutions)
{
    struct board_s b, c;
    struct board_choices_s board_choices;
    for (int i = 0; i < max_solutions ; i++)
        init_board(&solutions[i]);
    // Catch corner cases
    if (is_complete(board)) {
        solutions[0] = *board;
        solutions[0].solved = true;
        return 1;
    };
    if (is_broken(board, false)) {
        solutions[0] = *board;
        solutions[0].broken = true;
        return 0;
    }
    b = *board;
    fill_possibles_continuously(&b);
    int n = 0;
    for (int i = 0; i < BOARD_SIZE && n < max_solutions; i++) {
        if (num_bits(b.cells[i]) > 1) {
            for (int j = 1; j < BLOCK_SIZE + 1 && n < max_solutions; j++) {
                if (b.cells[i] & masks[j]) {
                    c = b;
                    c.cells[i] = masks[j];
                    board_choices = fill_cells(&c, false);
                    if (board_choices.board.solved == true &&
                        memcmp(board_choices.board.cells,
                               solutions[0].cells,
                               BOARD_SIZE * sizeof(uint64_t))) {
                        solutions[n] = board_choices.board;
                        ++n;
                    }
                }
            }
        }
    }
    return n;
}

int inc_counter(int counter[], int max_counter[])
{
    int i = 0;
    for (i = BOARD_SIZE - 1; i >= 0; i--) {
        if (counter[i] != max_counter[i]) {
            ++counter[i];
            for (int j=i+1; j < BOARD_SIZE; j++)
                counter[j] = 1;
            break;
        }
    }
    return i;
}

void generate(struct board_s board, int n)
{
    int counter[BOARD_SIZE];
    int max_counter[BOARD_SIZE];
    int i = BOARD_SIZE - 1;
    char puzzle[BOARD_SIZE + 1];

    fill_possibles_continuously(&board);
    if (is_broken(&board, true)) return;

    for (int i = 0; i < BOARD_SIZE; i++) {
        counter[i] = 0;
        max_counter[i] = num_bits(board.cells[i]) - 1;
    }

    // The second part of this condition will not be reached in our lifetimes
    // but it's there for completeness
    while(n > 0 && memcmp(counter, max_counter, sizeof(counter)) != 0) {
        struct board_s b = board;
        int j = get_nth_set_bit(b.cells[i], counter[i]) + 1;
        b.cells[i] = masks[j];
        struct board_choices_s bc = fill_cells(&b, false);
        convert_board_str(&bc.board, puzzle);
        if (bc.board.solved) {
            printf("%d,%lu,%s\n", n, bc.choices,puzzle);
            --n;
        } else {
            printf("F,%s\n", puzzle);
        }
        i = inc_counter(counter, max_counter);
    }
}

void test()
{
    int errors = 0;
    {
        const uint64_t inp[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        const uint64_t out[] = {0,1,1,2,1,2,2,3,1,2,2, 3, 2, 3, 3, 4};

        printf("Bit count:\n");
        for (int i = 0; i < sizeof(inp) / sizeof(inp[0]); i++) {
            int j = num_bits(inp[i]);
            if (j != out[i]) {
                printf("Bit count err. Index %d Val %lu Expected %lu Actual %d\n",
                       i, inp[i], out[i], j);
                ++errors;
            }
        }
    }
    {
        const unsigned char inp[] = {0,1,2,3,4,5, 6, 7, 8,  9};
        const uint64_t out[] = {0,1,2,4,8,16,32,64,128,256};
        printf("Set only bit:\n");
        for (int i = 0; i < sizeof(inp) / sizeof(inp[0]); i++) {
            uint64_t j = set_only_bit(inp[i]);
            if (j != out[i]) {
                printf("Set bit err. Index %d Val %u Expected %lu Actual %lu\n",
                       i, inp[i], out[i], j);
                ++errors;
            }
        }
    }
    {
        const uint64_t inp[] = {0,1,2,4,8,16,32,64,128,256};
        const int out[] =      {0,1,2,3,4,5, 6, 7, 8,  9};
        printf("Get set bit:\n");
        for (int i = 0; i < sizeof(inp) / sizeof(inp[0]); i++) {
            int j = get_set_bit_pos(inp[i]);
            if (j != out[i]) {
                printf("Get set pos err. Index %d Val %lu Expected %d Actual %d\n",
                       i, inp[i], out[i], j);
                ++errors;
            }
        }
    }
    {
        char *s;
        const uint64_t inp[] = {0,1,2,3,4,5,6,7,8,9,10,11,12};
        const char *out[] =    {"", "1", "2", "12" ,"3",
            "13", "23", "123","4", "14","24","124", "34"};
        printf("Get set bits (plural):\n");
        for (int i = 0; i < sizeof(inp) / sizeof(inp[0]); i++) {
            s = get_set_bits(inp[i]);
            int j = strlen(s);
            if (strncmp(s, out[i], BLOCK_SIZE) != 0) {
                printf("Get set bits err."
                       "Index %d Val %lu Expected %s Actual %s Bit set %d\n",
                       i, inp[i], out[i], s, j);
                ++errors;
            }
        }
    }
    {
        printf("Get block bits:\n");
        const char *almost = "0007201930174832066231594875612789343729645180"
             "49315762235897641786541329194632875";
        struct board_s board = convert_str_board(almost);
        const size_t *l = lookup[0];
        uint64_t r = get_block_bits(&board, 0, rows[l[0]]);
        uint64_t b = get_block_bits(&board, 0, blocks[l[1]]);
        uint64_t c = get_block_bits(&board, 0, cols[l[2]]);
        uint64_t x = (~(r | b | c)) & all_ones;
        int j = num_bits(r);
        if (j != 5) {
            printf("Get block bits err r. Expected 5. Actual %d\n", j);
            ++errors;
        }
        j = num_bits(b);
        if (j != 5) {
            printf("Get block bits err b. Expected 5. Actual %d\n", j);
            ++errors;
        }
        j = num_bits(c);
        if (j != 6) {
            printf("Get block bits err b. Expected 6. Actual %d\n", j);
            ++errors;
        }
        char *s = get_set_bits(x);
        if (strcmp(s, "48") != 0) {
            printf("Get block bits err. Expected 48. Actual %s\n", s);
            ++errors;
        }
    }
    {
        printf("Get nth set bit:\n");
        int i;
        uint64_t val = 0b101010101;
        i = get_nth_set_bit(val, 0);
        if (i != 0) {
            printf("Get nth set bit err. Expected 0. Actual %d\n", i);
            ++errors;
        }
        i = get_nth_set_bit(val, 1);
        if (i != 2) {
            printf("Get nth set bit err. Expected 2. Actual %d\n", i);
            ++errors;
        }
        i = get_nth_set_bit(val, 2);
        if (i != 4) {
            printf("Get nth set bit err. Expected 4. Actual %d\n", i);
            ++errors;
        }
        i = get_nth_set_bit(val, 3);
        if (i != 6) {
            printf("Get nth set bit err. Expected 6. Actual %d\n", i);
            ++errors;
        }
        i = get_nth_set_bit(val, 4);
        if (i != 8) {
            printf("Get nth set bit err. Expected 8. Actual %d\n", i);
            ++errors;
        }
        i = get_nth_set_bit(val, 5);
        if (i != -1) {
            printf("Get nth set bit err. Expected -1. Actual %d\n", i);
            ++errors;
        }
        printf("Set nth bit to 0:\n");
        val = set_bit_to_zero(val, 0);
        if (val != 0b101010100) {
            printf("Set nth bit to 0. Expected 0b101010100. Actual %s\n",
                   binary(val, 0));
            ++errors;
        }
        val = set_bit_to_zero(val, 8);
        if (val != 0b001010100) {
            printf("Set nth bit to 0. Expected 0b001010100. Actual %s\n",
                   binary(val, 0));
            ++errors;
        }
    }
    {
        const char *almost = "4587261939174832066231594875612789343729645180"
            "49315762235897641786541329194632875";
        printf("Conversions, completions and validity:\n");
        char puzzle[BOARD_SIZE + 1];
        struct board_s board;
        board = convert_str_board(almost);
        convert_board_str(&board, puzzle);
        if (strncmp(puzzle, almost, BOARD_SIZE) != 0)
            printf("Conversion err %d. Expected %s. Actual %s\n", __LINE__,
                   almost, puzzle);
        bool b = is_complete(&board);
        if (b) {
            printf("Is complete err %d. Expected %d. Actual %d\n",
                   __LINE__, !b, b);
            ++errors;
        }
        b = is_broken(&board, false);
        if (b) {
            printf("Is broken err %d. Expected %d. Actual %d\n",
                   __LINE__, !b, b);
            ++errors;
        }
        const char *complete = "458726193917483256623159487561278934372964518"
            "849315762235897641786541329194632875";
        board = convert_str_board(complete);
        b = is_complete(&board);
        if (!b) {
            printf("Is complete err %d. Expected %d. Actual %d\n",
                   __LINE__, !b, b);
            ++errors;
        }
        const char *broken = "458726193917483256623159487561278934372964518"
            "849315762235897641786541329194632857";
        board = convert_str_board(broken);
        b = is_broken(&board, false);
        if (!b) {
            printf("Is broken err %d. Expected %d. Actual %d\n",
                   __LINE__, !b, b);
            ++errors;
        }
    }
    {
        printf("Get possibles:\n");
        const char *empty =
            "000000000000000000000000000000000000000000000000000000000000000000000000000000000";
        struct board_s board = convert_str_board(empty);
        fill_possibles(&board);
        for (int i = 0; i < BOARD_SIZE; i++)
            if (board.cells[i] != 511) {
                printf("Get possibles err. Expected 45. Val %lu for index %d\n",
                       board.cells[i], i);
                ++errors;
            }
        const char *almost = "0007201930174000066230594875612789343729645000"
             "09315760035897641706041300004630000";
        board = convert_str_board(almost);
        fill_possibles(&board);
        printf("Printing board without possibles.\n");
        print_board(&board, false);
        printf("Printing board with possibles.\n");
        print_board(&board, true);
        for (int i = 0; i < BOARD_SIZE; i++) {
            int j = num_bits(board.cells[i]);
            if (i == 0 || i == 45 || i == 46 || i == 64 || i == 72 || i == 73) {
                if (j != 2) {
                    printf("Get possibles err. Expected 2 bits."
                           "Got %d for index %d\n", j, i);
                    ++errors;
                }
            } else if (i == 1) {
                if(j != 3) {
                    printf("Get possibles err. Expected 3 bits."
                           "Got %d for index %d\n", j, i);
                    ++errors;
                }
            } else if (j != 1) {
                printf("Get possibles err. Expected 1 bit. Got %d for index %d\n",
                       j, i);
                ++errors;
            }
        }
    }
    {
        printf("Fill cells:\n");
        const char *empty =
            "123456789000000000000000000000000000000000000000000000000000000000000000000000000";
        struct board_s board = convert_str_board(empty);
        struct board_choices_s bc = fill_cells(&board, false);
        if (is_broken(&bc.board, true)) {
            printf("Fill cells err. Board broken.\n");
            print_board(&bc.board, true);
            ++errors;
        }
        if (is_complete(&bc.board) == false) {
            printf("Fill cells err. Board incomplete.\n");
            print_board(&bc.board, true);
            ++errors;
        }
        board = convert_str_board(empty);
        bc = fill_cells(&board, true);
        if (is_broken(&bc.board, true)) {
            printf("Fill cells random err. Board broken.\n");
            print_board(&bc.board, true);
            ++errors;
        }
        if (is_complete(&bc.board) == false) {
            printf("Fill cells random err. Board incomplete.\n");
            print_board(&bc.board, true);
            ++errors;
        }
    }
    {
        printf("Solve:\n");
        const char *empty =
            "123456789000000000000000000000000000000000000000000000000000000000000000000000000";
        struct board_s board = convert_str_board(empty);
        struct board_s solutions[2];
        int n = solve(&board, solutions, 2);
        if (n != 2) {
            printf("Solve err. Expected 2 solutions. Actual %d\n", n);
            ++errors;
        }
        printf("Easy\n");
        const char *easy =
            "600000150000020000730805940042100090070986020090004310015702069000090000084000005";
        board = convert_str_board(easy);
        n = solve(&board, solutions, 2);
        if (n != 1) {
            printf("Solve err. Expected 1 solution. Actual %d\n", n);
            ++errors;
        }
        printf("Hard\n");
        const char *hard =
            "300900000105000700090000061080010900007020000000000340000000006000078090000304870";
        board = convert_str_board(hard);
        n = solve(&board, solutions, 2);
        if (n != 1) {
            printf("Solve err. Expected 1 solution. Actual %d\n", n);
            ++errors;
        }
        printf("Very Hard\n");
        const char *very_hard =
            "000040300040975800800000004070020400000601000005090020600000009009362040007080000";
        board = convert_str_board(very_hard);
        n = solve(&board, solutions, 2);
        if (n != 1) {
            printf("Solve err. Expected 1 solution. Actual %d\n", n);
            ++errors;
        }
        printf("Extremely Hard\n");
        const char *extremely_hard =
            "800000000003600000070090200050007000000045700000100030001000068008500010090000400";
        board = convert_str_board(extremely_hard);
        n = solve(&board, solutions, 2);
        if (n != 1) {
            printf("Solve err. Expected 1 solution. Actual %d\n", n);
            ++errors;
        }
        print_board(&solutions[0], true);
    }
    printf("Errors: %d\n", errors);
}

int
main(int argc, char *argv[])
{
    int c, option_index;
    char puzzle[BOARD_SIZE + 2];
    struct board_s board, solutions[2];
    const char *empty =
        "123456789000000000000000000000000000000000000000000000000000000000000000000000000";

    long seed = 0;
    srand48_r(seed, &rng_buf);

    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, options,
                        long_options, &option_index);
        if (c == -1)
            break;
        switch(c) {
        case 's':
            strncpy(puzzle, optarg, BOARD_SIZE+1);
            board = convert_str_board(puzzle);
            solve(&board, solutions, 2);
            if (solutions[0].solved) print_board(&solutions[0], false);
            if (solutions[1].solved) print_board(&solutions[1], false);
            if (solutions[0].broken) printf("Broken\n");
            if (solutions[1].solved) printf("Multiple\n");
            if (solutions[0].solved) {
                convert_board_str(&solutions[0], puzzle);
                printf("1,%s\n", puzzle);
            }
            if (solutions[1].solved) {
                convert_board_str(&solutions[1], puzzle);
                printf("2,%s\n", puzzle);
            }
            break;
        case 'g':
            board = convert_str_board(empty);
            generate(board, atoi(optarg));
            break;
        case 'v':
            break;
        case 'e':
            break;
        case 'm':
            break;
        case 'r':
            seed = atoi(optarg);
            srand48_r(seed, &rng_buf);
            break;
        case 't':
            printf("Testing\n");
            test();
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
