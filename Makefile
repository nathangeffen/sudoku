CC=gcc
release: sudoku.c sudoku.h
	$(CC) -Wall -O3 sudoku.c -o sudoku

debug: 
	$(CC) -Wall -g sudoku.c -o sudoku-debug

clean:
	rm sudoku sudoku-debug

