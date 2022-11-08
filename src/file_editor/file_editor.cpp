// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

/*Removes the program from the screen and exits.*/
void FileEditor::die(const char* s) {
    write(STDOUT_FILENO, TERM_CLEAR_SCREEN, 4);
    write(STDOUT_FILENO, TERM_SEND_CURSOR_HOME, 3);

    perror(s);
}

/* Init */
FileEditor::FileEditor(std::string dir_path) {
    E.statusmsg[0] = '\0';
    complete_path = dir_path;
    this->createFileList();
}

// Destructor functions

void FileEditor::clearFileList() {
    int x = 0;
    while (file_list.size > x) {
        free(file_list.p[x].filename);
        free(file_list.p[x].path);
        x += 1;
    }
    free(file_list.p);
}

// Does not seem to be called
FileEditor::~FileEditor() {
    free(buffer.b);
    editorFlushRows();
    clearFileList();
    if (this->no_gui) {
        disableRawMode();
    }
}

/* Main function
Doesnt run if the loop if test mode */
void FileEditor::runtime(bool loop) {
    this->no_gui = loop;

    if (this->no_gui) {
        enableRawMode();

        if (getWindowSize(&screenrows, &screencols) == -1) {
            die("getWindowSize");
        }
    }
    // Compensate for having a status bar
    screenrows -= 2;
    editorOpen(file_list.p[file_number].path);
    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit");
    while (this->no_gui && loop) {
        editorRefreshScreen();
        loop = editorProcessKeypress();
    }
}
