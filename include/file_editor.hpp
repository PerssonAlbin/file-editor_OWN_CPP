// Copyright 2022 Albin Persson
#ifndef INCLUDE_FILE_EDITOR_HPP_
#define INCLUDE_FILE_EDITOR_HPP_



#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#ifdef WINDOWS
    #include <direct.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define _BSD_SOURCE

#define CTRL_KEY(k) ((k) & 0x1f)
#define TAB_STOP 4
#define QUIT_TIMES 2
namespace fs = std::filesystem;

class FileEditor {
 private:
    // Append buffer
    struct abuf {
        char *b = reinterpret_cast<char*>(malloc(0));
        int len = 0;
    };
    abuf buffer;

    /* Cursor handling */
    struct cursor {
        int x = 0;
        int y = 0;
        int rx = 0;  // Position correction for tabs
    };
    cursor c;

    struct termios orig_termios;

    int file_number = 0;
    std::string complete_path;
    struct paths {
        char* filename;
        char* path;
        int size;
    };
    struct file_path_list {
        paths* p = reinterpret_cast<paths*>(malloc(0));
        int size = 0;
    };
    file_path_list file_list;

    /* Terminal handling */
    typedef struct erow {
        int rsize = 0;
        int size = 0;
        char *chars;
        char *render;
    } erow;
    struct editorConfig {
        int numrows = 0;
        erow *row = NULL;
        int rowoff = 0;
        int coloff = 0;
        char statusmsg[80];
        time_t statusmsg_time = 0;
        int dirty = 0;
    };
    editorConfig E;
    int screenrows;
    int screencols;

    /* Input handling */
    enum editorKey {
        BACKSPACE = 127,
        ARROW_LEFT = 3330,
        ARROW_RIGHT,
        ARROW_UP,
        ARROW_DOWN,
        DEL_KEY,
        HOME_KEY,
        END_KEY,
        PAGE_UP,
        PAGE_DOWN
    };

    /* Init functions */
    void die(const char *s);
    void clearFileList();

    /* Buffer Functions */
    void bufferAppend(const char *s, int len);
    char* editorRowToString(int* buflen);
    void editorUpdateRow(erow *row);
    void resetRows();
    void editorFlushRows();
    void editorFreeRow(erow *row);
    void editorInsertRow(int at, char *s, size_t len);
    void editorRowInsertChar(int at, int input);
    void editorInsertChar(int read_key);
    void editorRowDelChar(erow *row, int at_x, int at_y);
    void editorDelChar();
    void editorDelRow(int at);
    void editorRowAppendString(erow *row, char *s, size_t len);
    void editorInsertNewline();

    /* File functions */
    void createFileList();
    bool isDirectory(std::string path);
    void editorSave();
    void editorOpen(char *filename);

    /* Terminal functions */
    void enableRawMode();
    void disableRawMode();
    int getWindowSize(int *rows, int *cols);
    void editorSetStatusMessage(const char* fmt, ...);
    int editorReadKey();
    bool editorProcessKeypress();
    int getCursorPosition(int *rows, int *cols);

    /* Editor functions */
    void editorRefreshScreen();
    void editorDrawMessageBar();
    void editorDrawRows();
    void editorDrawStatusBar();
    void editorMoveCursor(int key);
    int editorRowCxToRx();
    void editorScroll();

    /* ANSI Escape Sequence for terminals */
    inline static const char* TERM_CLEAR_SCREEN = "\x1b[2J";
    inline static const char* TERM_SEND_CURSOR_HOME = "\x1b[H";

 public:
    // Init
    explicit FileEditor(std::string argv);

    // Destructor
    ~FileEditor();

    // Main function
    void runtime();
};

#endif  // INCLUDE_FILE_EDITOR_HPP_
