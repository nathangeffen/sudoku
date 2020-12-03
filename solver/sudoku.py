import argparse
import random
from operator import add
import time
import sys

ROWS = [
    [0,   1,  2,  3,  4,  5,  6,  7,  8 ],
    [9,  10, 11, 12, 13, 14, 15, 16, 17 ],
    [18, 19, 20, 21, 22, 23, 24, 25, 26 ],
    [27, 28, 29, 30, 31, 32, 33, 34, 35 ],
    [36, 37, 38, 39, 40, 41, 42, 43, 44 ],
    [45, 46, 47, 48, 49, 50, 51, 52, 53 ],
    [54, 55, 56, 57, 58, 59, 60, 61, 62 ],
    [63, 64, 65, 66, 67, 68, 69, 70, 71 ],
    [72, 73, 74, 75, 76, 77, 78, 79, 80 ]
]

BLOCKS = [
    [0, 1, 2,
     9, 10, 11,
     18, 19, 20],
    [3, 4, 5,
     12, 13, 14,
     21, 22, 23],
    [6, 7, 8,
     15, 16, 17,
     24, 25, 26],
    [27, 28, 29,
     36, 37, 38,
     45, 46, 47],
    [30, 31, 32,
     39, 40, 41,
     48, 49, 50],
    [33, 34, 35,
     42, 43, 44,
     51, 52, 53],
    [54, 55, 56,
     63, 64, 65,
     72, 73, 74],
    [57, 58, 59,
     66, 67, 68,
     75, 76, 77],
    [60, 61, 62,
     69, 70, 71,
     78, 79, 80]
]

COLS = [
    [0, 9, 18, 27, 36, 45, 54, 63, 72],
    [1, 10, 19, 28, 37, 46, 55, 64, 73],
    [2, 11, 20, 29, 38, 47, 56, 65, 74],
    [3, 12, 21, 30, 39, 48, 57, 66, 75],
    [4, 13, 22, 31, 40, 49, 58, 67, 76],
    [5, 14, 23, 32, 41, 50, 59, 68, 77],
    [6, 15, 24, 33, 42, 51, 60, 69, 78],
    [7, 16, 25, 34, 43, 52, 61, 70, 79],
    [8, 17, 26, 35, 44, 53, 62, 71, 80]
]

LOOKUP = [
    [0, 0, 0], [0, 0, 1], [0, 0, 2], [0, 1, 3], [0, 1, 4], [0, 1, 5],
    [0, 2, 6], [0, 2, 7], [0, 2, 8], [1, 0, 0], [1, 0, 1], [1, 0, 2],
    [1, 1, 3], [1, 1, 4], [1, 1, 5], [1, 2, 6], [1, 2, 7], [1, 2, 8],
    [2, 0, 0], [2, 0, 1], [2, 0, 2], [2, 1, 3], [2, 1, 4], [2, 1, 5],
    [2, 2, 6], [2, 2, 7], [2, 2, 8], [3, 3, 0], [3, 3, 1], [3, 3, 2],
    [3, 4, 3], [3, 4, 4], [3, 4, 5], [3, 5, 6], [3, 5, 7], [3, 5, 8],
    [4, 3, 0], [4, 3, 1], [4, 3, 2], [4, 4, 3], [4, 4, 4], [4, 4, 5],
    [4, 5, 6], [4, 5, 7], [4, 5, 8], [5, 3, 0], [5, 3, 1], [5, 3, 2],
    [5, 4, 3], [5, 4, 4], [5, 4, 5], [5, 5, 6], [5, 5, 7], [5, 5, 8],
    [6, 6, 0], [6, 6, 1], [6, 6, 2], [6, 7, 3], [6, 7, 4], [6, 7, 5],
    [6, 8, 6], [6, 8, 7], [6, 8, 8], [7, 6, 0], [7, 6, 1], [7, 6, 2],
    [7, 7, 3], [7, 7, 4], [7, 7, 5], [7, 8, 6], [7, 8, 7], [7, 8, 8],
    [8, 6, 0], [8, 6, 1], [8, 6, 2], [8, 7, 3], [8, 7, 4], [8, 7, 5],
    [8, 8, 6], [8, 8, 7], [8, 8, 8]
]

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def is_good_partial(board):
    for i in range(len(board)):
        if board[i] == 0:
            continue
        l = LOOKUP[i]
        rows, blocks, cols = ROWS[l[0]], BLOCKS[l[1]], COLS[l[2]]
        for group in [rows, blocks, cols]:
            for j in group:
                if i == j:
                    continue
                if board[i] == board[j]:
                    return False
    return True

def is_good_board(board):
    if len([b for b in board if b == 0]) > 0:
        return False
    return check_good_partial_board(board)


def product(l):
    total = 1
    for i in l:
        total *= i
    return total

def print_board(board):
    for i in range(81):
        if i == 0:
            pass
        elif i % 27 == 0:
            print()
            print()
        elif i % 9 == 0:
            print()
        elif i % 3 == 0:
            print(" ", end="")
        print(board[i], end="")
    print()

def get_impossibles(board, index, region):
    impossibles = [0] * 9
    for i in region:
        if i == index:
            continue
        if board[i] > 0:
            impossibles[board[i] - 1] = 1
    return impossibles

def get_possibles(board, index):
    possibles = [0] * 9
    result = []
    l = LOOKUP[index]
    rows, blocks, cols = ROWS[l[0]], BLOCKS[l[1]], COLS[l[2]]
    possibles = list(map(add, possibles, get_impossibles(board, index, rows)))
    possibles = list(map(add, possibles, get_impossibles(board, index, blocks)))
    possibles = list(map(add, possibles, get_impossibles(board, index, cols)))
    for i in range(len(possibles)):
        if possibles[i] == 0:
            result.append(i + 1)
    return result

def fill_forced(board, possibles, blocks):
    for i in range(1, 10):
        for block in blocks:
            if len([board[j] for j in block if board[j] == i]) == 0:
                selected = [p for p in possibles if p[0] in block]
                indices = [p[0] for p in selected if i in p[1]]
                if len(indices) == 1:
                    board[indices[0]] = i
    return board

def fill_ones(board, possibles):
    length_one = [r for r in possibles if len(r) == 1]
    for l in length_one:
        board[l[0]] = l[1][0]
        possibles.remove(l)
    return board, possibles

def filter_twos(possibles):
    for blocks in [ROWS, BLOCKS, COLS]:
        for block in blocks:
            selected = [p for p in possibles if p[0] in block]
            twosies = [p for p in selected if len(p[1]) == 2]
            if len(twosies) > 1:
                for i in range(len(twosies) - 1):
                    matched = False
                    index1 = twosies[i][0]
                    index2 = None
                    for j in range(i + 1, len(twosies)):
                        if twosies[i][1] == twosies[j][1]:
                            index2 = twosies[j][0]
                            matched = True
                            break
                    if matched:
                        for s in selected:
                            if s[0] != index1 and s[0] != index2:
                                for j in twosies[i][1]:
                                    try:
                                        s[1].remove(j)
                                    except:
                                        pass
    return possibles

def filter_combinations(possibles):
    for length in range(2,8):
        for blocks in [ROWS, BLOCKS, COLS]:
            for block in blocks:
                selected = [p for p in possibles if p[0] in block]
                twosies = [p for p in selected if len(p[1]) == length]
                if len(twosies) > length - 1:
                    for i in range(len(twosies) - 1):
                        index1 = twosies[i][0]
                        matched = [index1]
                        for j in range(i + 1, len(twosies)):
                            if twosies[i][1] == twosies[j][1]:
                                matched.append(twosies[j][0])
                                if len(matched) == length:
                                    break
                        if len(matched) == length:
                            for s in selected:
                                if s[0] not in matched:
                                    for j in twosies[i][1]:
                                        try:
                                            s[1].remove(j)
                                        except:
                                            pass
    return possibles


def get_all_possibles(board):
    return  [[i, get_possibles(board, i)]
                  for i in range(len(board)) if board[i] == 0]

def is_complete(board):
    return (len([b for b in board if b == 0]) == 0)

def contains_empty(possibles):
    return len([p for p in possibles if len(p[1]) == 0])

def place_singles(board, possibles):
    new_board = board.copy()
    for p in possibles:
        if new_board[p[0]] == 0 and len(p[1]) == 1:
            new_board[p[0]] = p[1][0]
    return new_board

def place_only_options(board, possibles):
    new_board = board.copy()
    b_p = board.copy()
    for p in possibles:
        i = p[0]
        if b_p[i] == 0:
            b_p[i] = p[1]

    for n in range(1,10):
        for block in ROWS + BLOCKS + COLS:
            matches = []
            for i in block:
                if new_board[i] == n:
                    matches = []
                    break
                if new_board[i] == 0:
                    if n in b_p[i]:
                        matches.append(i)
            if len(matches) == 1:
                new_board[matches[0]] = n
    return new_board



def fill_cells(board, products):
    stack = []
    possibles = get_all_possibles(board)
    # [0]->board, [1]->num possible vals (for calc permutations, [2]->used vals
    stack.append([board.copy(), 1, []])

    # Loop while stack is valid and we top board_new on stack is incomplete
    while len(stack) > 0 and is_complete(stack[-1][0]) == False:
        board_new = stack[-1][0].copy() # Work with board_new at top of stack

        # Check that it's good
        if is_good_partial(board_new) == False:
            stack.pop()
            break

        possibles = get_all_possibles(board_new)
        # Remove tried values from possibles
        used_indices = stack[-1][2]
        for p in possibles:
            removals = [u[1] for u in used_indices if p[0] == u[0]]
            for r in removals:
                try:
                    p[1].remove(r)
                except:
                    pass

        if contains_empty(possibles): # This is a bad board
            stack.pop()
        else:
            # Choose the shortest entry in possibles. Then choose a random
            # val from that entry.
            # [0]->len, [1]->cell idx, [2]->possible vals
            lengths = sorted([(len(p[1]), p[0], p[1]) for p in possibles])
            cell = lengths[0][1]
            index = random.randrange(0, lengths[0][0])
            val = lengths[0][2][index]
            board_new[cell] = val
            stack[-1][2].append((cell, val,))
            stack.append([board_new.copy(), lengths[0][0], []])

    if len(stack) == 0:
        return [None, []]
    else:
        return [stack[-1][0], [s[1] for s in stack]]

def make_complete(board=None):
    if board is None:
        board = [0] * 81
    return fill_cells(board.copy(), [])

def calc_avg(board=None, iterations=100):
    t1 = time.time()
    if board is None:
        board = [0] * 81
    results = []
    for i in range(iterations):
        results.append(make_complete(board.copy()))
        print("Finished")
        if results[-1][0]:
            print_board(results[-1][0])
            print(i, product(results[-1][1]))
        else:
            print("Failed")
    t2 = time.time()
    print("Time taken:", t2 - t1)
    return results

def check_results(results):
    # check for failures
    failures = [r for r in results if r[0] is None]
    if len(failures) > 0:
        print("Failures:", len(failures))
    # Check for incorrect boards
    incorrects = [r for r in results if is_complete(r[0]) == False]
    if len(incorrects) > 0:
        print("Incorrects", len(incorrects))
    # Check for duplicates
    dups = []
    boards = sorted([r[0] for r in results])
    for i in range(1, len(boards)):
        if boards[i] == boards[i-1]:
            dups.append(boards[i])
    if len(dups) > 0:
        print("Duplicates", len(dups))

def find_solutions(board, max_solutions=2):
    solutions = []
    possibles = get_all_possibles(board)
    for p in possibles:
        for val in p[1]:
            board_new = board.copy()
            board_new[p[0]] = val
            solution = make_complete(board_new)[0]
            if solution and (solution not in solutions):
                solutions.append(solution)
            if len(solutions) == max_solutions:
                return solutions

    return solutions


def make_puzzle(symmetrical=True, max_removals=55, min_ones=2, simple=True):
    board = make_complete()[0]
    if symmetrical:
        removals = list(range(41))
    else:
        removals = list(range(80))
    random.shuffle(removals)

    board_copy = board.copy()
    for r in removals:
        prev = board_copy.copy()
        board_copy[r] = 0
        if symmetrical and r != 40:
            board_copy[80 - r] = 0
        possibles = get_all_possibles(board_copy)
        if len([p for p in possibles if len(p[1]) == 1]) == 0:
            break
        num_blanks = len([c for c in board_copy if c==0])
        if num_blanks > max_removals:
            break
        if num_blanks > min_ones:
            if len(find_solutions(board_copy)) > 1:
                break
            if len([p for p in possibles if len(p[1]) == 1]) < min_ones:
                break

    if simple:
        return prev
    else:
        if len(find_solutions(board_copy)) == 1:
            return board_copy
        else:
            eprint("Using simpler because harder has duplicate")
            return prev

def num_filled_in(board):
    return len([b for b in board if b != 0])

def board_to_str(board):
    return "".join([str(p) for p in board])

def str_to_board(s):
    board = list(s)
    return [int(b) for b in board]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Sudoku solver and generator')
    parser.add_argument("-s", "--solve",
                        dest="solve",
                        type=str,
                        default="",
                        help="Solve a Sudoku puzzle")
    parser.add_argument("-v", "--very_easy",
                        dest="very_easy",
                        action="store_true",
                        help="Make an easy Sudoku puzzle")
    parser.add_argument("-e", "--easy",
                        dest="easy",
                        action="store_true",
                        help="Make an easy Sudoku puzzle")
    parser.add_argument("-m", "--medium",
                        dest="medium",
                        action="store_true",
                        help="Make a medium difficulty Sudoku puzzle")
    parser.add_argument("-d", "--hard",
                        dest="hard",
                        action="store_true",
                        help="Make a hard Sudoku puzzle")

    args = parser.parse_args()

    if args.solve:
        board = str_to_board(args.solve)
        solutions = find_solutions(board)
        for s in solutions:
            print_board(s)
            print("Solution:", board_to_str(s))

    if args.very_easy:
        puzzle = make_puzzle(max_removals=55, min_ones=6, simple=True)
        while num_filled_in(puzzle) > 45:
            puzzle = make_puzzle(max_removals=55, min_ones=6, simple=True)
        print_board(puzzle)
        print("Puzzle:", board_to_str(puzzle))

    if args.easy:
        puzzle = make_puzzle(max_removals=55, min_ones=4, simple=True)
        while num_filled_in(puzzle) > 40:
            puzzle = make_puzzle(max_removals=55, min_ones=4, simple=True)
        print_board(puzzle)
        print("Puzzle:", board_to_str(puzzle))

    if args.medium:
        puzzle = make_puzzle(max_removals=55, min_ones=2, simple=True)
        while num_filled_in(puzzle) > 35:
            puzzle = make_puzzle(max_removals=55, min_ones=2, simple=True)
        print_board(puzzle)
        print("Puzzle:", board_to_str(puzzle))

    if args.hard:
        puzzle = make_puzzle(max_removals=55, min_ones=2, simple=False)
        while num_filled_in(puzzle) > 30:
            puzzle = make_puzzle(max_removals=55, min_ones=2, simple=False)
        print_board(puzzle)
        print("Puzzle:", board_to_str(puzzle))
