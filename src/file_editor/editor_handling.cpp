// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

/*Append the char strings in row and then adds ~ to mark end of file.*/
void FileEditor::editorDrawRows() {
    int y;
    for (y = 0; y < screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == screenrows / 3) {
                int padding = (screencols) / 2;
                if (padding) {
                    bufferAppend(L"~", 1);
                    padding--;
                }
                while (padding--)
                    bufferAppend(L" ", 1);
            } else {
                bufferAppend(L"~", 1);
            }
        } else {
            int len = E.rows[filerow].render.size() - E.coloff;
            if (len < 0)
                len = 0;
            if (len > screencols)
                len = screencols;
            bufferAppend(&E.rows[filerow].render[E.coloff], len);
        }
        bufferAppend(TERM_CLEAR_LINES, 4);
        bufferAppend(L"\r\n", 3);
    }
}

/*Appends the status bar with white background showing filename
and current_line/total_lines.*/
void FileEditor::editorDrawStatusBar() {
    bufferAppend(TERM_INVERSE_COLOR, 5);
    wchar_t status[80];
    wchar_t rstatus[80];

    int len = swprintf(status, sizeof(status), L"%.20s %s",
                       file_list.p[file_number].filename
                           ? file_list.p[file_number].filename
                           : "[No Name]",
                       E.dirty ? "(modified)" : "");
    int rlen = swprintf(rstatus, sizeof(rstatus), L"%d/%d", c.y + 1, E.numrows);
    if (len > screencols)
        len = screencols;
    bufferAppend(status, len);
    while (len < screencols) {
        if (screencols - len == rlen) {
            bufferAppend(rstatus, rlen);
            break;
        } else {
            bufferAppend(L" ", 1);
            len++;
        }
    }
    bufferAppend(TERM_RESET_STYLE, 4);
    bufferAppend(L"\r\n", 2);
}

/*Simply render message bar for displaying info.*/
void FileEditor::editorDrawMessageBar() {
    bufferAppend(TERM_CLEAR_LINES, 4);
    int msglen = wcslen(E.statusmsg);
    if (msglen > screencols)
        msglen = screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        bufferAppend(E.statusmsg, msglen);
}

/*Scrolls if the cursor is at the edge of the screen.*/
void FileEditor::editorScroll() {
    c.rx = 0;
    if (c.y < E.numrows) {
        c.rx = editorRowCxToRx();
    }
    // Vertical scrolling
    if (c.y < E.rowoff) {
        E.rowoff = c.y;
    }
    if (c.y >= E.rowoff + screenrows) {
        E.rowoff = c.y - screenrows + 1;
    }
    if (c.rx < E.coloff) {
        E.coloff = c.rx;
    }
    if (c.rx >= E.coloff + screencols) {
        E.coloff = c.rx - screencols + 1;
    }
}

/*Moves the cursor according to the input from editorReadKey.*/
void FileEditor::editorMoveCursor(int key) {
    /* Moves cursor based on cases it gets from */

    erow* row = (c.y >= E.numrows) ? NULL : &E.rows[c.y];
    switch (key) {
    case ARROW_LEFT:
        if (c.x != 0) {
            c.x--;
        } else if (c.y > 0) {
            c.y--;
            c.x = E.rows[c.y].chars.size();
        }
        break;
    case ARROW_RIGHT:
        if (row && c.x < row->chars.size()) {
            c.x++;
        } else if (row && c.x == row->chars.size()) {
            c.y++;
            c.x = 0;
        }
        break;
    case ARROW_UP:
        if (c.y != 0)
            c.y--;
        break;
    case ARROW_DOWN:
        if (c.y < E.numrows)
            c.y++;
        break;
    }
    row = (c.y >= E.numrows) ? NULL : &E.rows[c.y];
    int rowlen = row ? row->chars.size() : 0;
    if (c.x > rowlen)
        c.x = rowlen;
}

/*Compensates the vertical position of the cursor since tabs take
up more than 1 character space.*/
int FileEditor::editorRowCxToRx() {
    int rx = 0;
    int j;
    for (j = 0; j < c.x; j++) {
        if (E.rows[c.y].chars[j] == TAB)
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

/*Adds the default values and makes functions calls
the other info to be displayed.*/
void FileEditor::editorRefreshScreen() {
    editorScroll();

    bufferAppend(TERM_HIDE_CURSOR, 7);
    bufferAppend(TERM_SEND_CURSOR_HOME, 4);

    editorDrawRows();
    editorDrawStatusBar();
    editorDrawMessageBar();

    wchar_t buf[32];
    // Tracks cursor
    swprintf(buf, sizeof(buf), L"\x1b[%d;%dH", (c.y - E.rowoff) + 1,
             (c.rx - E.coloff) + 1);
    bufferAppend(buf, wcslen(buf));

    bufferAppend(TERM_SHOW_CURSOR, 7);

    write(STDOUT_FILENO, buffer.b, buffer.len);
    buffer.len = 0;
    buffer.b = reinterpret_cast<wchar_t*>(realloc(buffer.b, 0));
}
