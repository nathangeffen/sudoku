# Sudoku utilities

The *solver* folder contains a C program to generate and solve Sudoku
problems. It also has a nifty option to list every single completed Sudoku
puzzle in an ordered way, though you may need a few lifetimes for it to finish.
(It generates about 30000 puzzles per second on a single core on my laptop and
there are about 10^21 Sudoku puzzles.)

The README.md file in the *solver* folder explains how the C program works.

The *ui* folder contains JavaScript code and CSS for a web-based Sudoku user
interface.

The README.md file in the *ui* folder explains how to use the Javascript and CSS.

All the code is FLOSS. See the LICENSE file.
