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

    let undo_stacks = {};
    let redo_stacks = {};

    let start_time = new Date();
    let additional_ms = 0;

    ///////////////////////////

    // Functions with no side effects

    const getMobileOS = () => {
        var userAgent = navigator.userAgent || navigator.vendor || window.opera;

        // Windows Phone must come first because its UA also contains "Android"
        if (/windows phone/i.test(userAgent)) {
            return "Winphone";
        }

        if (/android/i.test(userAgent)) {
            return "Android";
        }

        // iOS detection
        // https://stackoverflow.com/questions/9038625/detect-if-device-is-ios#9039885
        if ([
            'iPad Simulator',
            'iPhone Simulator',
            'iPod Simulator',
            'iPad',
            'iPhone',
            'iPod'].includes(navigator.platform)
            // iPad on iOS 13 detection
            || (navigator.userAgent.includes("Mac") && "ontouchend" in document)) {
            return "iOS";
        }
        return "unknown";
    }


    const isMobile = () => {
        const os = getMobileOS();
        if (os === 'Android' || os === 'iOS' || os === "Winphone") return true;
        return false;
    }

    const isSmallHeight = () => {
        if (screen.height < 768) return true;
        return false;
    }

    const twoDigits = (val) => {
        let res = val.toString();
        if (res.length < 2) {
            res = "0" + res;
        }
        return res;
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

    const saveUndo = (sudoku_div_id, cell, undo_redo='u') => {
        let note = 0;
        if (!(sudoku_div_id in undo_stacks)) {
            undo_stacks[sudoku_div_id] = [];
            redo_stacks[sudoku_div_id] = [];
        }
        if (cell.classList.contains('sudoku-note')) note = 1;
        if (undo_redo === 'u') {
            undo_stacks[sudoku_div_id].push([getCellIndex(cell),
                                             cell.textContent, note]);
        } else {
            redo_stacks[sudoku_div_id].push([getCellIndex(cell),
                                             cell.textContent, note]);
        }
    }

    const undo = (sudoku_div_id, undo_redo='u') => {
        let stacks;
        if (undo_redo === 'u') {
            stacks = undo_stacks;
        } else {
            stacks = redo_stacks;
        }

        if (sudoku_div_id in stacks && stacks[sudoku_div_id].length > 0) {
            let cells = getCellsByDivId(sudoku_div_id);
            const info = stacks[sudoku_div_id].pop();
            if (undo_redo === 'u') {
                undo_redo = 'r';
            } else {
                undo_redo = 'u';
            }
            saveUndo(sudoku_div_id, cells[info[0]], undo_redo);

            cells[info[0]].textContent = info[1];
            if (info[2]) {
                cells[info[0]].classList.add('sudoku-note');
            } else {
                cells[info[0]].classList.remove('sudoku-note');
            }
            setActiveCell(sudoku_div_id, cells[info[0]]);
            setFontSize(cells[info[0]]);
        }
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
        let note_array = [];
        for (const cell of cells) {
            if (isProtectedCell(cell)) {
                cell_array.push([cell.textContent, true])
            } else {
                cell_array.push([cell.textContent, false])
            }
            if (cell.classList.contains('sudoku-note')) {
                note_array.push(1);
            } else {
                note_array.push(0);
            }
        }
        localStorage.setItem(sudoku_prefix + key,
                             JSON.stringify({'grid': cell_array,
                                             'notes': note_array}));
    }

    const setCell = (cell, value, protect = false) => {
        setCellValue(cell, value);
        if (protect) {
            cell.classList.add('sudoku-protected');
            cell.contentEditable = false;
        } else {
            cell.classList.remove('sudoku-protected');
            if (!isMobile()) {
                cell.contentEditable = true;
            }
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

    const setNotes = (sudoku_div_id, notes) => {
        let cells = getCellsByDivId(sudoku_div_id);
        for (let i = 0; i < cells.length; i++) {
            if (notes[i] == 1) {
                cells[i].classList.add('sudoku-note');
            } else {
                cells[i].classList.remove('sudoku-note');
            }
        }
    }


    const loadGrid = (sudoku_div_id_from, sudoku_div_id_to) => {
        const key = Sudoku.getPuzzleString(sudoku_div_id_from);
        const board = JSON.parse(localStorage.getItem(sudoku_prefix + key));
        if (board && 'grid' in board) {
            const grid = board['grid'];
            if (grid) {
                setGrid(sudoku_div_id_to, grid);
            } else {
                return false;
            }
        }
        if (board && 'notes' in board) {
            const notes = board['notes'];
            if (notes) {
                setNotes(sudoku_div_id_to, notes);
            }
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
        saveUndo(sudoku_div_id, cell);
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

    const setupTable = (sudoku_div_id) => {
        let cells = getCellsByDivId(sudoku_div_id);
        let nokeyboard = false;
        if (isMobile()) {
            nokeyboard = true;
        }
        for (let cell of cells) {
            if (nokeyboard) {
                cell.contentEditable = false;
                cell.addEventListener("click", function(e) {
                    setActiveCell(sudoku_div_id, e.target);
                });
            } else {
                if (!isProtectedCell(cell)) {
                    cell.contentEditable = true;
                }
                cell.addEventListener("focus", function(e) {
                    setActiveCell(sudoku_div_id, e.target);
                });
                cell.addEventListener('keydown', function(e) {
                    const c = String.fromCharCode(e.keyCode);
                    if (' 0123456789'.includes(c)) {
                        processInput(sudoku_div_id, e.target, c);
                        e.preventDefault();
                    } else if (![8,9,17,35,36,37,39,46].includes(e.keyCode)) {
                        // Only allow L/R arrow keys, delete, backspace, home, end
                        e.preventDefault();
                    }
                });
                cell.addEventListener('keyup', function() {
                    processCell(sudoku_div_id, cell);
                });
            }
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

    const toggleColors = (sudoku_div_id, btn) => {
        let div = document.getElementById(sudoku_div_id);
        if (div.classList.contains('sudoku-no-colors')) {
            div.classList.remove('sudoku-no-colors');
            markAllDuplicateCells(sudoku_div_id);
            if (btn) {
                //btn.textContent = 'Colors off';
            }
        } else {
            div.classList.add('sudoku-no-colors');
            unmarkAllDuplicateCells(sudoku_div_id);
            if (btn) {
                //btn.textContent = 'Colors on';
            }
        }
    }

    const clearGrid = (sudoku_div) => {
        let cells = getCellsByDiv(sudoku_div);
        for (let cell of cells) {
            if (!isProtectedCell(cell)) {
                clearCellText(cell);
            }
            cell.classList.remove('sudoku-note');
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
        setupTable(sudoku_div_id);
        setupDigits(sudoku_div_id);
        const grid = convertPuzzleStr(puzzle_str);
        setGrid(sudoku_div_id, grid);
        if (options.try_load) {
            loadGrid(sudoku_div_id, sudoku_div_id);
        }
        findAndSetActiveCell(sudoku_div_id);
        let sudoku_div = document.getElementById(sudoku_div_id);
        let restart_btn = sudoku_div.getElementsByClassName('sudoku-restart')[0];
        if (restart_btn) {
            restart_btn.addEventListener('click', function() {
                if (confirm('Are you sure you wish to restart')) {
                    clearGrid(sudoku_div);
                    saveGrid(sudoku_div_id);
                    undo_stacks[sudoku_div_id] = [];
                    redo_stacks[sudoku_div_id] = [];
                }
            });
        }
        let colors_btn = sudoku_div.getElementsByClassName('sudoku-colors')[0];
        if (colors_btn) {
            colors_btn.addEventListener('click', function(e) {
                toggleColors(sudoku_div_id, e.target);
            });
        }
        if (options.colors_on == false) {
            toggleColors(sudoku_div_id, colors_btn);
        }
        let note_btn = sudoku_div.getElementsByClassName('sudoku-note-btn')[0];
        if (note_btn) {
            note_btn.addEventListener('click', function(e) {
                if (active_cell && sudoku_div.contains(active_cell)) {
                    saveUndo(sudoku_div_id, active_cell);
                    active_cell.classList.toggle('sudoku-note');
                    saveGrid(sudoku_div_id);
                }
            });
        }
        let undo_btn = sudoku_div.getElementsByClassName('sudoku-undo-btn')[0];
        if (undo_btn) {
            undo_btn.addEventListener('click', function(e) {
                undo(sudoku_div_id, 'u');
            });
        }
        let redo_btn = sudoku_div.getElementsByClassName('sudoku-redo-btn')[0];
        if (redo_btn) {
            redo_btn.addEventListener('click', function(e) {
                undo(sudoku_div_id, 'r');
            });
        }
        let load_btn = sudoku_div.getElementsByClassName('sudoku-load-btn')[0];
        if (load_btn) {
            load_btn.addEventListener('click', function(e) {
                let puzzle_str = sudoku_div.getElementsByClassName(
                    'sudoku-load-input')[0].value;
                if (puzzle_str.length != 81) {
                    alert("Puzzle needs to be exactly 81 digits");
                    return;
                }
                for (let i = 0; i < puzzle_str.length; i++) {
                    if (!"0123456789".includes(puzzle_str[i])) {
                        alert("Only digits allowed in puzzle");
                        return;
                    }
                }
                const grid = convertPuzzleStr(puzzle_str);
                setGrid(sudoku_div_id, grid);
            });
        }
        let stop_watch;
        const changeTime = () => {
            let now = new Date();
            let seconds = parseInt((now - start_time + additional_ms) / 1000);
            let display_seconds = seconds % 60;
            let minutes = parseInt(seconds / 60);
            let hours = parseInt(seconds / 3600);
            stop_watch_span.getElementsByClassName('sudoku-hours')[0].
                textContent = twoDigits(hours) + ":";
            stop_watch_span.getElementsByClassName('sudoku-minutes')[0].
                textContent = twoDigits(minutes) + ":";
            stop_watch_span.getElementsByClassName('sudoku-seconds')[0].
                textContent = twoDigits(display_seconds);
        }
        let stop_watch_span = sudoku_div.
            getElementsByClassName('sudoku-stopwatch')[0];
        if (stop_watch_span) {
            stop_watch = setInterval(changeTime, 500);
            let pause_button = sudoku_div.getElementsByClassName('sudoku-pause')[0];
            pause_button.addEventListener("click", function(e) {
                if (pause_button.classList.contains('sudoku-stopwatch-play')) {
                    start_time = new Date();
                    pause_button.classList.remove('sudoku-stopwatch-play');
                    pause_button.textContent = '𝍪';
                    stop_watch = setInterval(changeTime, 500);
                } else {
                    clearInterval(stop_watch);
                    additional_ms = new Date() - start_time;
                    console.log("Additional ms", additional_ms);
                    pause_button.classList.add('sudoku-stopwatch-play');
                    pause_button.textContent = '▶';
                }
            });
        }
    }

    const insertTable = (sudoku_div) => {
        const innerhtml = '<p class="sudoku-incomplete">Puzzle completed</p> ' +
              '<table class="sudoku-table"> '+
              '<tr id="sudoku-tr-0"> '+
              '<td class="sudoku-td-0" >&nbsp;</td> '+
              '<td class="sudoku-td-1" >&nbsp;</td> '+
              '<td class="sudoku-td-2" >&nbsp;</td> '+
              '<td class="sudoku-td-3" >&nbsp;</td> '+
              '<td class="sudoku-td-4" >&nbsp;</td> '+
              '<td class="sudoku-td-5" >&nbsp;</td> '+
              '<td class="sudoku-td-6" >&nbsp;</td> '+
              '<td class="sudoku-td-7" >&nbsp;</td> '+
              '<td class="sudoku-td-8" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-1"> '+
              '<td class="sudoku-td-9" >&nbsp;</td> '+
              '<td class="sudoku-td-10" >&nbsp;</td> '+
              '<td class="sudoku-td-11" >&nbsp;</td> '+
              '<td class="sudoku-td-12" >&nbsp;</td> '+
              '<td class="sudoku-td-13" >&nbsp;</td> '+
              '<td class="sudoku-td-14" >&nbsp;</td> '+
              '<td class="sudoku-td-15" >&nbsp;</td> '+
              '<td class="sudoku-td-16" >&nbsp;</td> '+
              '<td class="sudoku-td-17" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-2"> '+
              '<td class="sudoku-td-18" >&nbsp;</td> '+
              '<td class="sudoku-td-19" >&nbsp;</td> '+
              '<td class="sudoku-td-20" >&nbsp;</td> '+
              '<td class="sudoku-td-21" >&nbsp;</td> '+
              '<td class="sudoku-td-22" >&nbsp;</td> '+
              '<td class="sudoku-td-23" >&nbsp;</td> '+
              '<td class="sudoku-td-24" >&nbsp;</td> '+
              '<td class="sudoku-td-25" >&nbsp;</td> '+
              '<td class="sudoku-td-26" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-3"> '+
              '<td class="sudoku-td-27" >&nbsp;</td> '+
              '<td class="sudoku-td-28" >&nbsp;</td> '+
              '<td class="sudoku-td-29" >&nbsp;</td> '+
              '<td class="sudoku-td-30" >&nbsp;</td> '+
              '<td class="sudoku-td-31" >&nbsp;</td> '+
              '<td class="sudoku-td-32" >&nbsp;</td> '+
              '<td class="sudoku-td-33" >&nbsp;</td> '+
              '<td class="sudoku-td-34" >&nbsp;</td> '+
              '<td class="sudoku-td-35" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-4"> '+
              '<td class="sudoku-td-36" >&nbsp;</td> '+
              '<td class="sudoku-td-37" >&nbsp;</td> '+
              '<td class="sudoku-td-38" >&nbsp;</td> '+
              '<td class="sudoku-td-39" >&nbsp;</td> '+
              '<td class="sudoku-td-40" >&nbsp;</td> '+
              '<td class="sudoku-td-41" >&nbsp;</td> '+
              '<td class="sudoku-td-42" >&nbsp;</td> '+
              '<td class="sudoku-td-43" >&nbsp;</td> '+
              '<td class="sudoku-td-44" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-5"> '+
              '<td class="sudoku-td-45" >&nbsp;</td> '+
              '<td class="sudoku-td-46" >&nbsp;</td> '+
              '<td class="sudoku-td-47" >&nbsp;</td> '+
              '<td class="sudoku-td-48" >&nbsp;</td> '+
              '<td class="sudoku-td-49" >&nbsp;</td> '+
              '<td class="sudoku-td-50" >&nbsp;</td> '+
              '<td class="sudoku-td-51" >&nbsp;</td> '+
              '<td class="sudoku-td-52" >&nbsp;</td> '+
              '<td class="sudoku-td-53" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-6"> '+
              '<td class="sudoku-td-54" >&nbsp;</td> '+
              '<td class="sudoku-td-55" >&nbsp;</td> '+
              '<td class="sudoku-td-56" >&nbsp;</td> '+
              '<td class="sudoku-td-57" >&nbsp;</td> '+
              '<td class="sudoku-td-58" >&nbsp;</td> '+
              '<td class="sudoku-td-59" >&nbsp;</td> '+
              '<td class="sudoku-td-60" >&nbsp;</td> '+
              '<td class="sudoku-td-61" >&nbsp;</td> '+
              '<td class="sudoku-td-62" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-7"> '+
              '<td class="sudoku-td-63" >&nbsp;</td> '+
              '<td class="sudoku-td-64" >&nbsp;</td> '+
              '<td class="sudoku-td-65" >&nbsp;</td> '+
              '<td class="sudoku-td-66" >&nbsp;</td> '+
              '<td class="sudoku-td-67" >&nbsp;</td> '+
              '<td class="sudoku-td-68" >&nbsp;</td> '+
              '<td class="sudoku-td-69" >&nbsp;</td> '+
              '<td class="sudoku-td-70" >&nbsp;</td> '+
              '<td class="sudoku-td-71" >&nbsp;</td> '+
              '</tr> '+
              '<tr class="sudoku-tr-8"> '+
              '<td class="sudoku-td-72" >&nbsp;</td> '+
              '<td class="sudoku-td-73" >&nbsp;</td> '+
              '<td class="sudoku-td-74" >&nbsp;</td> '+
              '<td class="sudoku-td-75" >&nbsp;</td> '+
              '<td class="sudoku-td-76" >&nbsp;</td> '+
              '<td class="sudoku-td-77" >&nbsp;</td> '+
              '<td class="sudoku-td-78" >&nbsp;</td> '+
              '<td class="sudoku-td-79" >&nbsp;</td> '+
              '<td class="sudoku-td-80" >&nbsp;</td> '+
              '</tr> '+
              '</table> ';
        sudoku_div.innerHTML += innerhtml;
    }

    const insertDigitButtons = (sudoku_div) => {
        const innerhtml =
              '<p class="sudoku-buttons"> '+
              '<button class="sudoku-btn sudoku-btn-0" value="0">X</button> ' +
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
        return '<button class="sudoku-restart" title="Clear board and restart">Clear</button>';
    }

    const colorsButtonHTML = () => {
        return '<button class="sudoku-colors" title="Toggle warnings">Warn</button>';
    }

    const noteButtonHTML = () => {
        return '<button class="sudoku-note-btn" title="Note">Note</button>';
    }

    const undoButtonHTML = () => {
        const html =  '<button class="sudoku-undo-btn" title="Undo">Undo</button>' +
              '<button class="sudoku-redo-btn" title="Redo">Redo</button>';
        return html;
    }

    const loadFormHTML = () => {
        const html = '<button class="sudoku-load-btn">Load</button>' +
              '<input class="sudoku-load-input"' +
              'type="text" minlength="81" maxlength="81">';
        return html;
    }

    const stopWatchHTML = () => {
        const html = '<span class="sudoku-stopwatch">'+
              '<span class="sudoku-hours">00:</span>' +
              '<span class="sudoku-minutes">00:</span>' +
              '<span class="sudoku-seconds">00</span>' +
              '<span class="sudoku-pause">𝍪</span></span>';
        return html;
    }

    const insertControlButtons = (sudoku_div, options) => {
        let innerhtml = '<p class="sudoku-control">';
        if (options.restart_button === true) {
            innerhtml += restartButtonHTML();
        }
        if (options.colors_button === true) {
            innerhtml += colorsButtonHTML();
        }
        if (options.note_button === true) {
            innerhtml += noteButtonHTML();
        }
        if (options.undo_button === true) {
            innerhtml += undoButtonHTML();
        }
        if (options.load_form === true) {
            innerhtml += loadFormHTML();
        }
        innerhtml += '</p><p class="sudoku-stopwatch-box">';
        if (options.stop_watch === true) {
            innerhtml += stopWatchHTML();
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
            colors_on: true,
            digit_buttons: true,
            restart_button: true,
            colors_button: true,
            note_button: true,
            undo_button: true,
            stop_watch: false,
            load_form: false
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
