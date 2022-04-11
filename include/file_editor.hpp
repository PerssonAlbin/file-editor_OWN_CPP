#ifndef FILE_EDITOR_H
#define FILE_EDITOR_H

#include "send_debug.hpp"

#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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

class FileEditor
{
private:
    /* Append buffer */
    struct abuf {
        char *b = (char*)malloc(0);
        int len = 0;
    };
    abuf buffer;
    void bufferAppend(const char *s, int len);

    SendDebug debug;
    /* Path handling */
    int file_number = 0;
    std::string path;
    struct paths 
    {
        char* path;
    };
    struct file_path_list
    {
        paths* p = (paths*)malloc(0);
        int size = 0;
    };
    file_path_list file_list;
    void createPath(int argc, std::string argv);
    void monitorFileList();
    bool doesPathExist();

    /* Terminal handling */
    typedef struct erow
    {
        int size;
        char *chars;
    } erow;
    struct editorConfig
    {
        int numrows = 0;
        erow *row = NULL;
        int rowoff = 0;
        int coloff = 0;
    };
    editorConfig E;
    int screenrows;
    int screencols;
    
    void enableRawMode();
    //void disableRawMode();
    int getWindowSize(int *rows, int *cols);
    void editorDrawRows();
    void editorAppendRow(char *s, size_t len);
    void editorScroll();

    /* Cursor handling */
    struct cursor {
        int x = 0;
        int y = 0;
    };
    cursor c;
    int getCursorPosition(int *rows, int *cols);
    void editorMoveCursor(int key);

    /* Input handling */
    enum editorKey {
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
    int editorReadKey();
    void editorProcessKeypress();
    void editorRefreshScreen();
    void editorOpen(char *filename);

public:
    /* Init */
    FileEditor(int argc, std::string argv);
    ~FileEditor();

    /* Main function */
    void runtime();
};

#endif
