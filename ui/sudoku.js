/* sudoku.js 1.0 */
"use strict";

(function (Sudoku) {

    const sudoku_prefix = 'Sudoku-ng-';

    const sudoku_rows = [
        [0,   1,  2,  3,  4,  5,  6,  7,  8 ],
        [9,  10, 11, 12, 13, 14, 15, 16, 17 ],
        [18, 19, 20, 21, 22, 23, 24, 25, 26 ],
        [27, 28, 29, 30, 31, 32, 33, 34, 35 ],
        [36, 37, 38, 39, 40, 41, 42, 43, 44 ],
        [45, 46, 47, 48, 49, 50, 51, 52, 53 ],
        [54, 55, 56, 57, 58, 59, 60, 61, 62 ],
        [63, 64, 65, 66, 67, 68, 69, 70, 71 ],
        [72, 73, 74, 75, 76, 77, 78, 79, 80 ]
    ];

    const sudoku_squares = [
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
    ];

    const sudoku_cols = [
        [0, 9, 18, 27, 36, 45, 54, 63, 72],
        [1, 10, 19, 28, 37, 46, 55, 64, 73],
        [2, 11, 20, 29, 38, 47, 56, 65, 74],
        [3, 12, 21, 30, 39, 48, 57, 66, 75],
        [4, 13, 22, 31, 40, 49, 58, 67, 76],
        [5, 14, 23, 32, 41, 50, 59, 68, 77],
        [6, 15, 24, 33, 42, 51, 60, 69, 78],
        [7, 16, 25, 34, 43, 52, 61, 70, 79],
        [8, 17, 26, 35, 44, 53, 62, 71, 80]
    ];

    const sudoku_lookup = [
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
    ];


    const DUPLICATE = {
        no: 0,
        many_to_many: 1,
        many_to_one: 2,
        one_to_many: 3,
        one_to_one: 4,
    }

    let active_cell = null;

    ///////////////////////////

    // Functions with no side effects

    const getMobileOS = () => {
        var userAgent = navigator.userAgent || navigator.vendor || window.opera;

        // Windows Phone must come first because its UA also contains "Android"
        if (/windows phone/i.test(userAgent)) {
            return "Windows Phone";
        }

        if (/android/i.test(userAgent)) {
            return "Android";
        }

        // iOS detection from: http://stackoverflow.com/a/9039885/177710
        if (/iPad|iPhone|iPod/.test(userAgent) && !window.MSStream) {
            return "iOS";
        }
        return "unknown";
    }

    const convertPuzzleStr = (str) => {
        let arr = []
        for (let i = 0; i < str.length; i++) {
            if (str[i] === "0") {
                arr.push(["&nbsp;", false]);
            } else {
                arr.push([str[i], true]);
            }
        }
        return arr;
    }

    const filterDigits = (value) => {
        let new_val = "";
        for (let i = 0; i < value.length; i++) {
            if ("123456789".includes(value[i]) &&
                !new_val.includes(value[i])) {
                new_val = new_val + value[i];
            }
        }
        return new_val;
    }

    const sortStr = (s) => {
        let arr = s.split("");
        let arr_sorted = arr.sort();
        return arr_sorted.join("");
    }

    const getCellsByTable = (sudoku_table) => {
        return sudoku_table.getElementsByTagName('td');
    }

    const getCellsByDiv = (sudoku_div) => {
        return getCellsByTable(sudoku_div.getElementsByTagName('table')[0]);
    }

    const getCellsByDivId = (sudoku_div_id) => {
        return getCellsByDiv(document.getElementById(sudoku_div_id));
    }

    const getCellIndex = (cell) => {
        const class_prefix = "sudoku-td-"
        for (const name of cell.classList) {
            if (name.includes(class_prefix)) {
                return parseInt(name.substr(class_prefix.length));
            }
        }
    }

    const getCellValue = (cell) => {
        const value = cell.textContent;
        if (value.trim().length > 0) {
            return value;
        } else {
            return "0";
        }
    }

    const isSet = (cell) => {
        if (getCellValue(cell) != "0") {
            return true;
        } else {
            return false;
        }
    }

    const digitIn = (cell, digit) => {
        if (cell.textContent.includes(digit)) {
            return true;
        } else {
            return false;
        }
    }

    const isDuplicateGroup = (cells, index, group) => {
        let one_to_many = false;
        let many_to_many = false;

        const val_1 = getCellValue(cells[index]);
        if (val_1 == "0") return DUPLICATE.no;

        for (const i of group) {
            const val_2 = getCellValue(cells[i]);
            if (index === i || val_2 === "0") continue; // same cell or cf. space
            if (val_1.length === 1) { // Check for one_to_one or one_to_many
                if (val_1 === val_2) {
                    return DUPLICATE.one_to_one;
                } else if (val_2.includes(val_1)) {
                    one_to_many = true;
                }
            } else { // Check for many_to_one or many_to_many
                if (val_2.length === 1 && val_1.includes(val_2)) {
                    return DUPLICATE.many_to_one;
                } else if (val_1 === val_2) {
                    many_to_many = true;
                }
            }
        }

        if (one_to_many) return DUPLICATE.one_to_many;
        if (many_to_many) return DUPLICATE.many_to_many;
        return DUPLICATE.no;
    }

    const isDuplicateCell = (cells, cell) => {
        let result_rows = DUPLICATE.no;
        let result_cells = DUPLICATE.no;
        let result_cols = DUPLICATE.no;
        const index = getCellIndex(cell);
        const lookup = sudoku_lookup[index];
        if (isSet(cell)) {
            result_rows = isDuplicateGroup(cells, index, sudoku_rows[lookup[0]]);
            result_cells = isDuplicateGroup(cells, index,
                                             sudoku_squares[lookup[1]]);
            result_cols = (isDuplicateGroup(cells, index, sudoku_cols[lookup[2]]));
        }
        return Math.max(result_rows, result_cells, result_cols);
    }

    Sudoku.checkCompleted = (sudoku_div_id) => {
        const cells = getCellsByDivId(sudoku_div_id);
        for (const cell of cells) {
            const val = getCellValue(cell);
            if (val.length != 1  || val === "0" ||
                isDuplicateCell(cells, cell)) {
                return false;
            }
        }
        return true;
    }

    const isProtectedCell = (cell) => {
        return cell.classList.contains('sudoku-protected');
    }

    Sudoku.getPuzzleString = (sudoku_div_id) => {
        let str = "";
        const cells = getCellsByDivId(sudoku_div_id);
        for (const cell of cells) {
            if (isProtectedCell(cell)) {
                str += getCellValue(cell);
            } else {
                str += "0";
            }
        }
        return str;
    }

    Sudoku.getCurrentPuzzleString = (sudoku_div_id) => {
        let str = "";
        const cells = getCellsByDivId(sudoku_div_id);
        for (const cell of cells) {
            const s = getCellValue(cell);
            if (s.length != 1) {
                str += "0";
            } else {
                str += s;
            }
        }
        return str;
    }

    const getAffectedCells = (cells, cell) => {
        let result = [];
        const index = getCellIndex(cell);
        const lookup = sudoku_lookup[index];
        const indices = sudoku_rows[lookup[0]].concat(
            sudoku_squares[lookup[1]]).concat(sudoku_cols[lookup[2]]);
        for (let i = 0; i < cells.length; i++) {
            if (indices.includes(i)) {
                result.push(cells[i]);
            }
        }
        return result;
    }

    // Functions with side effects

    // From https://stackoverflow.com/questions/1181700/set-cursor-position-on-contenteditable-div
    const placeCursorAtEnd = function(el) {
        var selection = window.getSelection();
        var range = document.createRange();
        selection.removeAllRanges();
        range.selectNodeContents(el);
        range.collapse(false);
        selection.addRange(range);
        el.focus();
    }

    const setCellValue = (cell, value) => {
        if (value === (" ") || value === "0" || value === "&nbsp;") {
            cell.innerHTML = "&nbsp;";
        } else {
            const v = sortStr(filterDigits(value));
            if (v.length > 0) {
                cell.textContent = v;
            } else {
                cell.innerHTML = "&nbsp;";
            }
        }
    }

    const removeCellDigit = (cell, digit) => {
        let value = cell.textContent;
        let new_value = "";
        for (let i = 0; i < value.length; i++) {
            if (value[i] != digit) {
                new_value += value[i];
            }
        }
        setCellValue(cell, new_value);
    }

    const markDuplicateCell = (cell, result) => {
        unmarkDuplicateCell(cell);
        if (result === DUPLICATE.many_to_many) {
            cell.classList.add('sudoku-many-to-many');
        } else if (result === DUPLICATE.many_to_one) {
            cell.classList.add('sudoku-many-to-one');
        } else if (result === DUPLICATE.one_to_many) {
            cell.classList.add('sudoku-one-to-many');
        } else if (result === DUPLICATE.one_to_one) {
            cell.classList.add('sudoku-one-to-one');
        }
    }

    const unmarkDuplicateCell = (cell) => {
        cell.classList.remove('sudoku-many-to-many');
        cell.classList.remove('sudoku-many-to-one');
        cell.classList.remove('sudoku-one-to-many');
        cell.classList.remove('sudoku-one-to-one');
    }

    const markAllDuplicateCells = (sudoku_div_id) => {
        const cells = getCellsByDivId(sudoku_div_id);
        for (let cell of cells) {
            let result = isDuplicateCell(cells, cell);
            if (result) {
                markDuplicateCell(cell, result)
            } else {
                unmarkDuplicateCell(cell);
            }
        }
    }

    const unmarkAllDuplicateCells = (sudoku_div_id) => {
        const cells = getCellsByDivId(sudoku_div_id);
        for (let cell of cells) {
            unmarkDuplicateCell(cell);
        }
    }

    const markCompleted = (sudoku_div_id) => {
        let elem = document.getElementById(sudoku_div_id);
        let tbl = elem.getElementsByTagName('table')[0];
        tbl.classList.add('sudoku-table-completed');
        let paras = elem.getElementsByClassName('sudoku-incomplete');
        for (let p of paras) {
            p.classList.remove('sudoku-incomplete');
            p.classList.add('sudoku-complete');
        }
    }

    const unmarkCompleted = (sudoku_div_id) => {
        let elem = document.getElementById(sudoku_div_id);
        let tbl = elem.getElementsByTagName('table')[0];
        tbl.classList.remove('sudoku-table-completed');
        let paras = elem.getElementsByClassName('sudoku-complete');
        for (let p of paras) {
            p.classList.add('sudoku-incomplete');
            p.classList.remove('sudoku-complete');
        }
    }

    const showIfCompleted = (sudoku_div_id) => {
        if (Sudoku.checkCompleted(sudoku_div_id)) {
            markCompleted(sudoku_div_id);
        } else {
            unmarkCompleted(sudoku_div_id);
        }
    }

    const saveGrid = (sudoku_div_id) => {
        const key = Sudoku.getPuzzleString(sudoku_div_id);
        const cells = getCellsByDivId(sudoku_div_id);
        let cell_array = [];
        for (const cell of cells) {
            if (isProtectedCell(cell)) {
                cell_array.push([cell.textContent, true])
            } else {
                cell_array.push([cell.textContent, false])
            }
        }
        localStorage.setItem(sudoku_prefix + key,
                             JSON.stringify(cell_array));
    }

    const setCell = (cell, value, protect = false) => {
        setCellValue(cell, value);
        if (protect) {
            cell.classList.add('sudoku-protected');
            cell.contentEditable = false;
        } else {
            cell.classList.remove('sudoku-protected');
            cell.contentEditable = true;
        }
    }

    const setGrid = (sudoku_div_id, grid) => {
        let cells = getCellsByDivId(sudoku_div_id);
        for (let i = 0; i < cells.length; i++) {
            setCell(cells[i], grid[i][0], grid[i][1]);
            setFontSize(cells[i]);
            if (cells[i].classList.contains('sudoku-protected')) {
                cells[i].addEventListener('click', function() {
                    highlightEqualDigits(sudoku_div_id, cells[i], true);
                });
            }
        }
    }

    const loadGrid = (sudoku_div_id_from, sudoku_div_id_to) => {
        const key = Sudoku.getPuzzleString(sudoku_div_id_from);
        const grid = JSON.parse(localStorage.getItem(sudoku_prefix + key));
        if (grid) {
            setGrid(sudoku_div_id_to, grid);
        } else {
            return false;
        }
        markAllDuplicateCells(sudoku_div_id_to);
        return true;
    }

    const setActiveCell = (sudoku_div_id, cell) => {
        if (active_cell) {
            active_cell.classList.remove('sudoku-td-in-focus');
        }
        active_cell = cell;
        active_cell.focus();
        cell.classList.add('sudoku-td-in-focus');
        highlightAffectedCells(sudoku_div_id, cell);
        highlightEqualDigits(sudoku_div_id, cell);
    }

    const setFontSize = (cell) => {
        cell.classList.remove('sudoku-font-1', 'sudoku-font-2',
                               'sudoku-font-3', 'sudoku-font-4',
                               'sudoku-font-5', 'sudoku-font-6',
                               'sudoku-font-7', 'sudoku-font-8',
                               'sudoku-font-9');
        const l = cell.textContent.length;
        const className = 'sudoku-font-' + l.toString();
        cell.classList.add(className);
    }

    const highlightAffectedCells = (sudoku_div_id, cell) => {
        const cells = getCellsByDivId(sudoku_div_id);
        for (let c of cells) {
            c.classList.remove('sudoku-in-scope');
        }
        let selected = getAffectedCells(cells, cell);
        for (let c of selected) {
            c.classList.add('sudoku-in-scope');
        }
    }

    const highlightEqualDigits = (sudoku_div_id, cell, samecell=false) => {
        const cells = getCellsByDivId(sudoku_div_id);
        for (let c of cells) {
            if (cell.textContent.length === 1 &&
                cell.textContent >= '1' &&
                cell.textContent <= '9' &&
                c.textContent === cell.textContent &&
                (samecell == true || c != cell)) {
                c.classList.add('sudoku-equal-digit');
            } else {
                c.classList.remove('sudoku-equal-digit');
            }
        }
    }

    const processCell = (sudoku_div_id, cell) => {
        markAllDuplicateCells(sudoku_div_id);
        saveGrid(sudoku_div_id);
        showIfCompleted(sudoku_div_id);
        setFontSize(cell);
        highlightEqualDigits(sudoku_div_id, cell);
    }

    const processInput = (sudoku_div_id, cell, value) => {
        if (value === '\xa0' || value === " " || value === "0") {
            setCellValue(cell, "0");
        } else if (digitIn(cell, value)) {
            removeCellDigit(cell, value);
        } else {
            setCellValue(cell, cell.textContent + value);
        }
        cell.focus();
        highlightEqualDigits(sudoku_div_id, cell);
        placeCursorAtEnd(cell);
    }

    const setupTable = (sudoku_div_id, sudoku_table) => {
        let cells = sudoku_table.getElementsByTagName('td');
        let blurCells = false;
        const os = getMobileOS();
        if (os === "Android" || os === "iOS") blurCells = true;
        for (let cell of cells) {
            if (blurCells) {
                cell.addEventListener("focus", function(e) {
                    cell.blur();
                });
            }
            cell.addEventListener("focus", function(e) {
                setActiveCell(sudoku_div_id, e.target);
            });
            cell.addEventListener('keydown', function(e) {
                const c = String.fromCharCode(e.keyCode);
                if (' 0123456789'.includes(c)) {
                    processInput(sudoku_div_id, e.target, c);
                    e.preventDefault();
            } else if (![8, 9, 17, 35, 36, 37, 39, 46].includes(e.keyCode)) {
                e.preventDefault();
            }
            });
            cell.addEventListener('keyup', function() {
                processCell(sudoku_div_id, cell);
            });
        }
    }


    const setupDigits = (sudoku_div_id) => {
        let sudoku_div = document.getElementById(sudoku_div_id);
        let digits =  sudoku_div.getElementsByClassName('sudoku-btn');
        for (const digit of digits) {
            digit.addEventListener("click", function(e) {
                e.preventDefault();
                if (active_cell && sudoku_div.contains(active_cell)) {
                    processInput(sudoku_div_id, active_cell, e.target.value);
                    processCell(sudoku_div_id, active_cell);
                }
            });
        }
    }

    const clearCellText = (cell) => {
        cell.innerHTML = "&nbsp;";
    }

    const toggleClues = (sudoku_div_id, btn) => {
        let div = document.getElementById(sudoku_div_id);
        if (div.classList.contains('sudoku-no-clues')) {
            div.classList.remove('sudoku-no-clues');
            markAllDuplicateCells(sudoku_div_id);
            if (btn) {
                btn.textContent = 'Clues off';
            }
        } else {
            div.classList.add('sudoku-no-clues');
            unmarkAllDuplicateCells(sudoku_div_id);
            if (btn) {
                btn.textContent = 'Clues on';
            }
        }
    }

    const clearGrid = (sudoku_div) => {
        let cells = getCellsByDiv(sudoku_div);
        for (let cell of cells) {
            if (!isProtectedCell(cell)) {
                clearCellText(cell);
            }
            unmarkDuplicateCell(cell);
        }
    };

    const findAndSetActiveCell = (sudoku_div_id) => {
        if (!active_cell) {
            let cells = getCellsByDivId(sudoku_div_id);
            for (const cell of cells) {
                if (cell.classList.contains('sudoku-protected')) {
                    continue;
                } else {
                    setActiveCell(sudoku_div_id, cell);
                    processCell(sudoku_div_id, cell);
                    //placeCursorAtEnd(cell);
                    return;
                }
            }
        }
    }

    const init = (sudoku_div_id, puzzle_str, options) => {
        let sudoku_div = document.getElementById(sudoku_div_id);
        let sudoku_table = sudoku_div.getElementsByTagName('table')[0];
        setupTable(sudoku_div_id, sudoku_table);
        setupDigits(sudoku_div_id);
        const grid = convertPuzzleStr(puzzle_str);
        setGrid(sudoku_div_id, grid);
        if (options.try_load) {
            loadGrid(sudoku_div_id, sudoku_div_id);
        }
        findAndSetActiveCell(sudoku_div_id);
        let restart_btn = sudoku_div.getElementsByClassName('sudoku-restart')[0];
        if (restart_btn) {
            restart_btn.addEventListener('click', function() {
                if (confirm('Are you sure you wish to restart')) {
                    clearGrid(sudoku_div);
                    saveGrid(sudoku_div_id);
                }
            });
        }
        let clues_btn = sudoku_div.getElementsByClassName('sudoku-clues')[0];
        if (clues_btn) {
            clues_btn.addEventListener('click', function(e) {
                toggleClues(sudoku_div_id, e.target);
            });
        }
        if (options.clues_on == false) {
            toggleClues(sudoku_div_id, clues_btn);
        }
    }

    const insertTable = (sudoku_div) => {
        const innerhtml = '<p class="sudoku-incomplete">Puzzle completed</p> ' +
              '<table class="sudoku-table"> '+
              '<tr id="sudoku-tr-0"> '+
              '<td class="sudoku-td-0" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-1" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-2" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-3" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-4" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-5" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-6" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-7" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-8" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-1"> '+
              '<td class="sudoku-td-9" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-10" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-11" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-12" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-13" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-14" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-15" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-16" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-17" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-2"> '+
              '<td class="sudoku-td-18" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-19" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-20" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-21" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-22" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-23" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-24" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-25" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-26" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-3"> '+
              '<td class="sudoku-td-27" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-28" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-29" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-30" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-31" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-32" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-33" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-34" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-35" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-4"> '+
              '<td class="sudoku-td-36" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-37" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-38" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-39" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-40" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-41" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-42" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-43" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-44" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-5"> '+
              '<td class="sudoku-td-45" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-46" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-47" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-48" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-49" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-50" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-51" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-52" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-53" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-6"> '+
              '<td class="sudoku-td-54" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-55" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-56" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-57" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-58" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-59" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-60" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-61" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-62" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-7"> '+
              '<td class="sudoku-td-63" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-64" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-65" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-66" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-67" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-68" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-69" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-70" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-71" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-8"> '+
              '<td class="sudoku-td-72" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-73" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-74" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-75" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-76" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-77" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-78" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-79" contenteditable=true>&nbsp;</td> '+
              '<td class="sudoku-td-80" contenteditable=true>&nbsp;</td> '+
              '</tr> '+
              '</table> ';
        sudoku_div.innerHTML += innerhtml;
    }


    const insertDigitButtons = (sudoku_div) => {
        const innerhtml =
              '<p class="sudoku-buttons"> '+
              '<button class="sudoku-btn sudoku-btn-0" value="0"><img src="eraser.svg" alt="eraser"/></button> ' +
              '<button class="sudoku-btn sudoku-btn-1" value="1">1</button> ' +
              '<button class="sudoku-btn sudoku-btn-2" value="2">2</button> ' +
              '<button class="sudoku-btn sudoku-btn-3" value="3">3</button> ' +
              '<button class="sudoku-btn sudoku-btn-4" value="4">4</button> ' +
              '<button class="sudoku-btn sudoku-btn-5" value="5">5</button> ' +
              '<button class="sudoku-btn sudoku-btn-6" value="6">6</button> ' +
              '<button class="sudoku-btn sudoku-btn-7" value="7">7</button> ' +
              '<button class="sudoku-btn sudoku-btn-8" value="8">8</button> ' +
              '<button class="sudoku-btn sudoku-btn-9" value="9">9</button> ' +
              '</p> ';
        sudoku_div.innerHTML += innerhtml;
    }

    const restartButtonHTML = () => {
        return '<button class="sudoku-restart">Restart</button>';
    }

    const cluesButtonHTML = () => {
        return '<button class="sudoku-clues">Clues off</button>';
    }

    const insertControlButtons = (sudoku_div, options) => {
        let innerhtml = '<p class="sudoku-control">';
        if (options.restart_button == true) {
            innerhtml += restartButtonHTML();
        }
        if (options.clues_button == true) {
            innerhtml += cluesButtonHTML();
        }
        innerhtml += '</p>';
        sudoku_div.innerHTML += innerhtml;
    }

    const insertHTML = (sudoku_div, options) => {
        insertTable(sudoku_div);
        if (options.digit_buttons == true) {
            insertDigitButtons(sudoku_div);
        }
        insertControlButtons(sudoku_div, options);
    }

    Sudoku.create = (sudoku_div_id, puzzle_str, options = {}) => {
        let default_options = {
            try_load: true,
            clues_on: true,
            digit_buttons: true,
            restart_button: true,
            clues_button: true
        };
        for (let [key, value] of Object.entries(options)) {
            if (key in default_options) {
                default_options[key] = value;
            } else {
                console.log('Unknown option:', key)
            }
        }
        let div = document.getElementById(sudoku_div_id);
        div.classList.add('sudoku-div');
        insertHTML(div, default_options);
        init(sudoku_div_id, puzzle_str, default_options);
    }
}(window.Sudoku = window.Sudoku || {}));
