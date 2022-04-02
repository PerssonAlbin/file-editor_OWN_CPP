#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <filesystem>
#include <termios.h>
#include <sys/ioctl.h>
#include "debug/send_debug.hpp"

#ifdef WINDOWS
    #include <direct.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define CTRL_KEY(k) ((k) & 0x1f)
#define EDITOR_VERSION "0.0.1"
namespace fs = std::filesystem;

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

    /* Path handling */
    std::string path;
    std::vector<std::string> file_list;
    void createPath(int argc, std::string argv);
    void monitorFileList();
    bool doesPathExist();

    /* Terminal handling */
    int screenrows;
    int screencols;
    struct termios orig_termios;
    void enableRawMode();
    void disableRawMode();
    int getWindowSize(int *rows, int *cols);
    void editorDrawRows();

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
        PAGE_UP,
        PAGE_DOWN
    };
    int editorReadKey();
    void editorProcessKeypress();
    void editorRefreshScreen();

    /*Error handling */
    void die(const char *s);
    
public:
    /* Init */
    FileEditor(int argc, std::string argv);
    ~FileEditor();

    /* Main function */
    void runtime();
};

/* Init */
FileEditor::FileEditor(int argc, std::string argv)
{
    this->createPath(argc, argv);
    //std::cout << "Path: " << path << "\n";
    if(doesPathExist())
    {
        this->monitorFileList();
    }
    else
    {
        throw std::invalid_argument("Path could not be found");
    }
}

FileEditor::~FileEditor()
{
    disableRawMode();
    free(buffer.b);
}

/* Main function */
void FileEditor::runtime()
{
    enableRawMode();
    if(getWindowSize(&screenrows, &screencols) == -1) die("getWindowSize");
    while(1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
}

/* Append buffer */
void FileEditor::bufferAppend(const char *s, int len)
{
    char *new_buffer = (char*)realloc(buffer.b, buffer.len + len);

    if(new_buffer == NULL) return;
    memcpy(&new_buffer[buffer.len], s, len);
    buffer.b = new_buffer;
    buffer.len += len;
}

/* Path handling */
void FileEditor::createPath(int argc, std::string argv)
{
    char cwd[256];
    if(argc == 1 || argc == 2 && argv == ".")
    {
        getcwd(cwd, sizeof(cwd));
        path = std::string(cwd);
    }
    else
    {
        char first_char_argv = argv[0];
        if(&first_char_argv == std::string("/"))
        {
            path = argv;
        }
        else
        {
            path = std::string(getcwd(cwd, sizeof(cwd))) + "/" + argv;
        }
    }
}

void FileEditor::monitorFileList()
{
    for (const auto & entry : fs::recursive_directory_iterator(this->path))
        file_list.insert(std::end(file_list), entry.path());
}

bool FileEditor::doesPathExist()
{
  struct stat buffer;
  return (stat (path.c_str(), &buffer) == 0);
}

/* Terminal handling */
void FileEditor::enableRawMode()
{
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void FileEditor::disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

int FileEditor::getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
      if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void FileEditor::editorDrawRows()
{
    int y;
    for (y = 0; y < screenrows; y++) {
        if (y == screenrows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "File editor -- version %s", EDITOR_VERSION);
            if (welcomelen > screencols) welcomelen = screencols;
            int padding = (screencols - welcomelen) / 2;
            if (padding) {
                bufferAppend("~", 1);
                padding--;
            }
            while (padding--) bufferAppend(" ", 1);
            bufferAppend(welcome, welcomelen);
        }
        else
        {
            bufferAppend("~", 1);
        }
        bufferAppend("\x1b[K", 3);
        if (y < screenrows - 1) {
            bufferAppend("\r\n", 2);
        }
    }
}

/* Cursor handling */
int FileEditor::getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
    return 0;
}

void FileEditor::editorMoveCursor(int key)
{
    switch (key)
    {
        case ARROW_LEFT:
            if(c.x != 0) c.x--;
            break;
        case ARROW_RIGHT:
            if(c.x != screencols -1) c.x++;
            break;
        case ARROW_UP:
            if(c.y != 0) c.y--;
            break;
        case ARROW_DOWN:
            if(c.y != screencols -1) c.y++;
            break;
    }
}

/* Input handling */
int FileEditor::editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    //Special case for arrow keys
    if (c == '\x1b')
    {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        }
        return '\x1b';
    }
    else
    {
        return c;
    }
}
 
void FileEditor::editorProcessKeypress()
{
    int c = editorReadKey();
    int times = screenrows;
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);

            exit(0);
            break;
        case PAGE_UP:
        case PAGE_DOWN:
            debug("Running pagedown");
            while (times--)
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
    }
    
}

void FileEditor::editorRefreshScreen()
{
    bufferAppend("\x1b[?25l", 6);
    bufferAppend("\x1b[H", 3);

    editorDrawRows();

    char buf[32];
    //Tracks cursor
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", c.y + 1, c.x + 1);
    bufferAppend(buf, strlen(buf));

    bufferAppend("\x1b[?25h", 6);

    write(STDOUT_FILENO, buffer.b, buffer.len);
    buffer.len = 0;
    //May be wrong
    buffer.b = (char*)realloc(buffer.b, 0);
}

/* Error handling */
void FileEditor::die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}