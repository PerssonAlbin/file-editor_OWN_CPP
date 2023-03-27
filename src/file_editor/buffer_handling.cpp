// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

/*
Append buffer
Arguments:
Char string to add to buffer
The size of the char string
*/
void FileEditor::bufferAppend(const char* s, int len) {
    char* new_buffer =
        reinterpret_cast<char*>(realloc(buffer.b, buffer.len + len));

    if (new_buffer == NULL) {
        return;
    }
    memcpy(&new_buffer[buffer.len], s, len);
    buffer.b = new_buffer;
    buffer.len += len;
}

std::vector<std::string> FileEditor::editorRowToString(int& buflen) {
    int j;
    std::vector<std::string> buf;
    for (j = 0; j < E.numrows; j++) {
        buf.push_back(E.rows[j].chars);
        buflen += E.rows[j].chars.size();
    }
    return buf;
}

/*Updates the row char string to identify and modify the display of tabs.*/
void FileEditor::editorUpdateRow(erow* row) {
    int tabs = 0;
    int j;
    for (j = 0; j < row->chars.size(); j++) {
        if (row->chars[j] == TAB)
            tabs++;
    }

    // free(row->render);
    row->render = "";
    SyntaxHighlight syntax;
    std::string syntaxed_row =
        syntax.hightlightLine(row->chars, file_list.p[file_number].filename);

    // row->render = reinterpret_cast<char*>(
    //     malloc(syntaxed_row.size() + tabs * (TAB_STOP - 1) + 1));
    int idx = 0;

    for (j = 0; j < syntax.added_length + row->chars.size(); j++) {
        if (syntaxed_row[j] == TAB) {
            row->render.push_back(' ');
            while (idx % TAB_STOP != 0) {
                row->render.push_back(' ');
            }
        } else {
            row->render.push_back(syntaxed_row[j]);
        }
    }
    row->render.push_back(END_OF_ROW);
}

/*Appends a char string to a char string inside a row.*/
void FileEditor::editorInsertRow(int at, std::string s) {
    if (at < 0 || at > E.numrows) {
        return;
    }
    // Extends the amount of rows by one
    if (at == E.rows.size()) {
        erow temp_row = {s, ""};
        E.rows.push_back(temp_row);
    } else {
        // Erases from the "old" row so that that only whats before the mouse
        // remains
        E.rows[at - 1].chars.erase(E.rows[at - 1].chars.size() - s.size(),
                                   s.size());
        // Adds a new row with the string to the right of the mouse
        erow temp_row = {s, ""};
        E.rows.insert(E.rows.begin() + at, temp_row);
    }

    E.rows[at].render = "";
    editorUpdateRow(&E.rows[at]);

    E.numrows++;
    E.dirty++;
}

void FileEditor::editorRowInsertChar(int at, int input) {
    if (at < 0 || at > E.rows[c.y].chars.size()) {
        at = E.rows[c.y].chars.size();
    }
    char c_input = input;
    std::string str_input = &c_input;
    E.rows[c.y].chars.insert(at, str_input);

    // E.rows[c.y].size++;
    editorUpdateRow(&E.rows[c.y]);
    E.dirty++;
}

void FileEditor::editorInsertChar(int read_key) {
    if (c.y == E.numrows) {
        editorInsertRow(E.numrows, const_cast<char*>(""));
    }
    editorRowInsertChar(c.x, read_key);
    c.x++;
}

/*Resets all the settings from the previous file*/
void FileEditor::resetRows() {
    editorFlushRows();
    E.numrows = 0;
    E.coloff = 0;
    E.rowoff = 0;
    c.x = 0;
    c.rx = 0;
    c.y = 0;
}

/* Frees one row */
void FileEditor::editorFreeRow(erow* row) {
    row->chars = "";
    row->render = "";
}

/*Frees all the buffered rows */
void FileEditor::editorFlushRows() {
    int x = 0;
    while (x < E.numrows) {
        editorFreeRow(&E.rows[x]);
        x += 1;
    }
    E.rows.clear();
}

void FileEditor::editorDelRow(int at) {
    if (at < 0 || at >= E.numrows) {
        return;
    }
    editorFreeRow(&E.rows[at]);
    memmove(&E.rows[at], &E.rows[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

void FileEditor::editorRowDelChar(erow* row, int at_x, int at_y) {
    if (at_x < 0 || at_x >= row->chars.size()) {
        return;
    }
    memmove(&row->chars[at_x], &row->chars[at_x + 1], row->chars.size() - at_x);
    // row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

void FileEditor::editorDelChar() {
    if (c.y == E.numrows) {
        return;
    }
    if (c.x == 0 && c.y == 0) {
        return;
    }
    erow* row = &E.rows[c.y];
    if (c.x > 0) {
        editorRowDelChar(row, c.x - 1, c.y);
        c.x--;
    } else {
        c.x = E.rows[c.y - 1].chars.size();
        editorRowAppendString(&E.rows[c.y - 1], row->chars);
        editorDelRow(c.y);
        c.y--;
    }
}

void FileEditor::editorRowAppendString(erow* row, std::string s) {
    row->chars.insert(row->chars.size(), s);
    row->chars[row->chars.size()] = END_OF_ROW;
    editorUpdateRow(row);
    E.dirty++;
}

void FileEditor::editorInsertNewline() {
    if (c.x == 0) {
        editorInsertRow(c.y, const_cast<char*>(""));
    } else {
        erow* row = &E.rows[c.y];
        editorInsertRow(c.y + 1, &row->chars[c.x]);
        row = &E.rows[c.y];
        row->chars[row->chars.size()] = END_OF_ROW;
        editorUpdateRow(row);
    }
    c.y++;
    c.x = 0;
}
