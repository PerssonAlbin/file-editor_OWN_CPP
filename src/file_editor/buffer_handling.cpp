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

std::vector<std::string> FileEditor::editorRowToString(int* buflen) {
    // int totlen = 0;
    int j;
    // for (j = 0; j < E.numrows; j++)
    //     totlen += E.row[j].size + 1;
    // *buflen = totlen;

    // char* buf = reinterpret_cast<char*>(malloc(totlen));
    std::vector<std::string> buf;
    // char* p = buf;
    for (j = 0; j < E.numrows; j++) {
        // memcpy(p, E.row[j].chars, E.row[j].size);
        // buf p += E.row[j].size;
        // *p = '\n';
        // p++;
        buf.push_back(E.row[j].chars);
    }
    return buf;
}

/*Updates the row char string to identify and modify the display of tabs.*/
void FileEditor::editorUpdateRow(erow* row) {
    int tabs = 0;
    int j;
    row->size = row->chars.size();
    for (j = 0; j < row->chars.size(); j++) {
        if (row->chars[j] == TAB)
            tabs++;
    }

    free(row->render);
    SyntaxHighlight syntax;
    std::string syntaxed_row =
        syntax.hightlightLine(row->chars, file_list.p[file_number].filename);

    row->render = reinterpret_cast<char*>(
        malloc(syntaxed_row.size() + tabs * (TAB_STOP - 1) + 1));
    int idx = 0;
    row->rsize = syntax.added_length + row->size;
    for (j = 0; j < row->rsize; j++) {
        if (syntaxed_row[j] == TAB) {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) {
                row->render[idx++] = ' ';
            }
        } else {
            row->render[idx++] = syntaxed_row[j];
        }
    }
    row->render[idx] = END_OF_ROW;
    row->rsize = idx;
}

/*Appends a char string to a char string inside a row.*/
void FileEditor::editorInsertRow(int at, std::string s, size_t len) {
    if (at < 0 || at > E.numrows) {
        return;
    }
    // Extends the amount of rows by one
    erow temp_row = {0, 0, s, NULL};
    E.row.push_back(temp_row);
    // E.row =
    //    reinterpret_cast<erow*>(realloc(E.row, sizeof(erow) * (E.numrows +
    //    1)));
    // Moves the rows 1 step down from the current position
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));

    // In case the row is cut off in the middle this makes sure the length
    // reflects that
    E.row[at].size = len;
    // E.row[at].chars = s;
    //  E.row[at].chars = reinterpret_cast<char*>(malloc(len + 1));
    //  Copies from the mouse x point and forward len characters
    //  memcpy(E.row[at].chars, s, len);
    //  E.row[at].chars[len] = END_OF_ROW;

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

void FileEditor::editorRowInsertChar(int at, int input) {
    if (at < 0 || at > E.row[c.y].size) {
        at = E.row[c.y].size;
    }
    // E.row[c.y].chars =
    //      reinterpret_cast<char*>(realloc(E.row[c.y].chars, E.row[c.y].size +
    //      2));

    // memmove(&E.row[c.y].chars[at + 1], &E.row[c.y].chars[at],
    //          E.row[c.y].size - at);
    char c_input = input;
    std::string str_input = &c_input;
    E.row[c.y].chars.insert(at, str_input);

    E.row[c.y].size++;
    // E.row[c.y].chars[at] = input;
    editorUpdateRow(&E.row[c.y]);
    E.dirty++;
}

void FileEditor::editorInsertChar(int read_key) {
    if (c.y == E.numrows) {
        editorInsertRow(E.numrows, const_cast<char*>(""), 0);
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
    // free(row->chars);
    row->chars = "";
    free(row->render);
}

/*Frees all the buffered rows */
void FileEditor::editorFlushRows() {
    int x = 0;
    while (x < E.numrows) {
        editorFreeRow(&E.row[x]);
        x += 1;
    }
    // free(E.row);
    // E.row = NULL;
}

void FileEditor::editorDelRow(int at) {
    if (at < 0 || at >= E.numrows) {
        return;
    }
    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    E.numrows--;
    E.dirty++;
}

void FileEditor::editorRowDelChar(erow* row, int at_x, int at_y) {
    if (at_x < 0 || at_x >= row->size) {
        return;
    }
    memmove(&row->chars[at_x], &row->chars[at_x + 1], row->size - at_x);
    row->size--;
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
    erow* row = &E.row[c.y];
    if (c.x > 0) {
        editorRowDelChar(row, c.x - 1, c.y);
        c.x--;
    } else {
        c.x = E.row[c.y - 1].size;
        editorRowAppendString(&E.row[c.y - 1], row->chars, row->size);
        editorDelRow(c.y);
        c.y--;
    }
}

void FileEditor::editorRowAppendString(erow* row, std::string s, size_t len) {
    // row->chars =
    //     reinterpret_cast<char*>(realloc(row->chars, row->size + len + 1));
    // memcpy(&row->chars[row->size], s, len);
    row->chars.insert(row->size, s);
    row->size += len;
    row->chars[row->size] = END_OF_ROW;
    editorUpdateRow(row);
    E.dirty++;
}

void FileEditor::editorInsertNewline() {
    if (c.x == 0) {
        editorInsertRow(c.y, const_cast<char*>(""), 0);
    } else {
        erow* row = &E.row[c.y];
        editorInsertRow(c.y + 1, &row->chars[c.x], row->size - c.x);
        row = &E.row[c.y];
        row->size = c.x;
        row->chars[row->size] = END_OF_ROW;
        editorUpdateRow(row);
    }
    c.y++;
    c.x = 0;
}
