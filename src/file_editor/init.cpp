// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

/*Removes the program from the screen and exits.*/
void FileEditor::die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
}

/* Init */
FileEditor::FileEditor(std::string argv) {
    E.statusmsg[0] = '\0';
    complete_path = argv;
    if (pathExist()) {
        this->createFileList();
    } else {
        throw std::invalid_argument("Path could not be found");
    }
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
    disableRawMode();
}

/* Main function */
void FileEditor::runtime() {
    enableRawMode();
    if (getWindowSize(&screenrows, &screencols) == -1) die("getWindowSize");
    // Compensate for having a status bar
    screenrows -= 2;
    editorOpen(file_list.p[file_number].path);
    editorSetStatusMessage("HELP: Ctrl-Q = quit");
    bool loop = true;
    while (loop) {
        editorRefreshScreen();
        loop = editorProcessKeypress();
    }
}
