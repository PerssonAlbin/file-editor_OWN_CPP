// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

/*
Append buffer
Arguments:
Char string to add to buffer
The size of the char string
*/
void FileEditor::bufferAppend(const char *s, int len) {
    char *new_buffer = reinterpret_cast<char*>(
        realloc(buffer.b, buffer.len + len));

    if (new_buffer == NULL) return;
    memcpy(&new_buffer[buffer.len], s, len);
    buffer.b = new_buffer;
    buffer.len += len;
}

char* FileEditor::editorRowToString(int* buflen) {
    int totlen = 0;
    int j;
    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size + 1;
    *buflen = totlen;

    char* buf = reinterpret_cast<char*>(
        malloc(totlen));
    char* p = buf;
    for (j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

/*Updates the row char string to identify and modify the display of tabs.*/
void FileEditor::editorUpdateRow(erow *row, int at) {
    int tabs = 0;
    // int at = E.numrows;
    int j;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') tabs++;
    }

    free(E.row[at].render);
    row->render = reinterpret_cast<char*>(
        malloc(row->size + tabs*(TAB_STOP - 1) + 1));
    int idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

/*Appends a char string to a char string inside a row.*/
void FileEditor::editorAppendRow(char *s, size_t len) {
    E.row = reinterpret_cast<erow*>(
        realloc(E.row, sizeof(erow) * (E.numrows + 1)));
    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = reinterpret_cast<char*>(malloc(len + 1));
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at], at);
    E.numrows++;
    E.dirty++;
}

void FileEditor::editorRowInsertChar(int at, int input) {
    if (at < 0 || at > E.row[c.y].size) at = E.row[c.y].size;
    E.row[c.y].chars = reinterpret_cast<char*>(
        realloc(E.row[c.y].chars, E.row[c.y].size + 2));
    memmove(&E.row[c.y].chars[at + 1],
        &E.row[c.y].chars[at], E.row[c.y].size - at);
    E.row[c.y].size++;
    E.row[c.y].chars[at] = input;
    editorUpdateRow(&E.row[c.y], c.y);
    E.dirty++;
}

void FileEditor::editorInsertChar(int read_key) {
    if (c.y == E.numrows) {
        editorAppendRow(const_cast<char*>(""), 0);
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

/*Frees all the buffered rows */
void FileEditor::editorFlushRows() {
    int x = 0;
    while (x < E.numrows) {
        free(E.row[x].chars);
        free(E.row[x].render);
        x += 1;
    }
    free(E.row);
    E.row = NULL;
}

void FileEditor::editorRowDelChar(erow *row, int at_x, int at_y) {
    if (at_x < 0 || at_x >= row->size) return;
    memmove(&row->chars[at_x],
        &row->chars[at_x + 1],
        row->size - at_x);
    row->size--;
    editorUpdateRow(row, at_y);
    E.dirty++;
}

void FileEditor::editorDelChar() {
    if (c.y == E.numrows) return;
    erow *row = &E.row[c.y];
    if (c.x > 0) {
        editorRowDelChar(row, c.x - 1, c.y);
        c.x--;
    }
}