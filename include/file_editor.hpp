// Copyright 2022 Albin Persson
#ifndef INCLUDE_FILE_EDITOR_HPP_
#define INCLUDE_FILE_EDITOR_HPP_

#include "include/file_type.hpp"
#include "include/syntax_highlight.hpp"
#include <climits>
#include <clocale>
#include <codecvt>
#include <cstdlib>
#include <cuchar>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <vector>

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
namespace ft = filetype;

class FileEditor {
  private:
    // Append buffer
    struct abuf {
        wchar_t* b = reinterpret_cast<wchar_t*>(malloc(0));
        int len = 0;
    };
    abuf buffer;

    /* Cursor handling */
    struct cursor {
        int x = 0;
        int y = 0;
        int rx = 0; // Position correction for tabs
    };
    cursor c;

    struct termios orig_termios;

    int file_number = 0;
    std::string complete_path;
    struct paths {
        char* filename;
        char* path;
        ft::FileType filetype;
        int size;
    };
    struct file_path_list {
        paths* p = reinterpret_cast<paths*>(malloc(0));
        int size = 0;
    };
    file_path_list file_list;

    /* Terminal handling */
    typedef struct erow {
        std::wstring chars;
        std::wstring render;
    } erow;
    struct editorConfig {
        int numrows = 0;
        std::vector<erow> rows;
        int rowoff = 0;
        int coloff = 0;
        wchar_t statusmsg[80];
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
    void die(const char* s);
    void clearFileList();

    /* Helper functions */
    std::string wstringToString(std::wstring wstring_to_conv);

    /* Buffer Functions */
    void bufferAppend(const wchar_t* s, int len);
    std::vector<std::string> editorRowToString(int& buflen);
    void editorUpdateRow(erow* row);
    void resetRows();
    void editorFlushRows();
    void editorFreeRow(erow* row);
    void editorInsertRow(int at, std::wstring s);
    void editorRowInsertChar(int at, int input);
    void editorInsertChar(int read_key);
    void editorRowDelChar(erow* row, int at_x, int at_y);
    void editorDelChar();
    void editorDelRow(int at);
    void editorRowAppendString(erow* row, std::wstring s);
    void editorInsertNewline();

    /* File functions */
    void createFileList();
    bool isDirectory(std::string path);
    std::string trimFilename(std::string filename, int length);
    void editorSave();
    void editorOpen(char* filename);

    /* Terminal functions */
    void enableRawMode();
    void disableRawMode();
    int getWindowSize(int* rows, int* cols);
    void editorSetStatusMessage(const wchar_t* fmt, ...);
    int editorReadKey();
    bool editorProcessKeypress();
    std::string decimalToHex(int decimalValue);
    std::string getUnicodeString(const std::string& hex);
    std::string codepoint_to_utf8(char32_t codepoint);
    int getCursorPosition(int* rows, int* cols);

    /* Editor functions */
    void editorRefreshScreen();
    void editorDrawMessageBar();
    void editorDrawRows();
    void editorDrawStatusBar();
    void editorMoveCursor(int key);
    int editorRowCxToRx();
    void editorScroll();

    /* ANSI Escape Sequence for terminals */
    inline static const wchar_t END_OF_ROW = L'\0';
    inline static const wchar_t TERM_ESC = L'\x1b';
    inline static const wchar_t TAB = L'\t';
    inline static const wchar_t* TERM_CLEAR_LINES = L"\x1b[K";
    inline static const wchar_t* TERM_RESET_STYLE = L"\x1b[m";
    inline static const wchar_t* TERM_INVERSE_COLOR = L"\x1b[7m";
    inline static const wchar_t* TERM_CLEAR_SCREEN = L"\x1b[2J";
    inline static const wchar_t* TERM_SEND_CURSOR_HOME = L"\x1b[H";
    inline static const wchar_t* TERM_SHOW_CURSOR = L"\x1b[?25h";
    inline static const wchar_t* TERM_HIDE_CURSOR = L"\x1b[?25l";

  public:
    // Init
    explicit FileEditor(std::string argv);

    // Destructor
    ~FileEditor();

    // Main function
    void runtime();
};

#endif // INCLUDE_FILE_EDITOR_HPP_
