# Sudoku Web User Interface

The example.js file shows how to use this library.

You only need sudoku.js and sudoku.css. You can also replace sudoku.css with
your own styles.

In your HTML head put this line:

    <link rel="stylesheet" href="sudoku.css">

Somewhere in your HTML file define an empty div with an identifier. E.g.

    <div id="sudoku-1"></div>

Somewhere after this in your HTML file put this line:

    <script src="sudoku.js"></script>

To create a puzzle:

    "use strict";
    const hard = '000005001004600200030970040189000000450020098000000314090064020008007900700300000';

    Sudoku.create('sudoku-1', hard);


You can pass options to the Sudoku.create function. E.g.

    Sudoku.create('sudoku-1', hard, {try_load: true,
                                     clues_on: false,
                                     clues_button: false});

The options are:

- try_load: Tries to load the user's filled in blocks from localStorage (defaults to true)

- clues_on: Provides the user with color-coded clues (defaults to true)

- clues_button: Provides the user with a button to toggle clues (defaults to true)

- digit_buttons: Provides the user with a button for each digit (defaults to
  true)

- restart_button: Provides the user with a button to restart from the original
  position (defaults to true)
