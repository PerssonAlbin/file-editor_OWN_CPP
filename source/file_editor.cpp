// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

#define _BSD_SOURCE

#define CTRL_KEY(k) ((k) & 0x1f)
#define EDITOR_VERSION "0.0.1"
#define TAB_STOP 4
namespace fs = std::filesystem;

struct termios orig_termios;

/*Removes the program from the screen and exits.*/
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
}

/* Init */
FileEditor::FileEditor(int argc, std::string argv) {
    E.statusmsg[0] = '\0';
    formatPath(argc, argv);
    if (doesPathExist()) {
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

/*
Append buffer
Arguments:
Char string to add to buffer
The size of the char string
*/
void FileEditor::bufferAppend(const char *s, int len) {
    char *new_buffer = (char*)realloc(buffer.b, buffer.len + len);

    if (new_buffer == NULL) return;
    memcpy(&new_buffer[buffer.len], s, len);
    buffer.b = new_buffer;
    buffer.len += len;
}

// Path handling
/*Fixes the path taken as argument when starting the program.*/
void FileEditor::formatPath(int argc, std::string argv) {
    char cwd[256];
    if (argc == 1 || (argc == 2 && argv == ".")) {
        getcwd(cwd, sizeof(cwd));
        complete_path = std::string(cwd);
    } else {
        char first_char_argv = argv[0];
        if (&first_char_argv == std::string("/")) {
            complete_path = argv;
        } else {
            complete_path = std::string(getcwd(cwd, sizeof(cwd))) + "/" + argv;
        }
    }
}

/*Iterates through the current path until all files and
directories have been added to file_list.*/
void FileEditor::createFileList() {
    std::string placeholder;
    int len;
    for (const auto & entry :
        fs::recursive_directory_iterator(this->complete_path)) {
        placeholder = entry.path();
        len = placeholder.size();
        if (isDirectory(placeholder)) {
            // Means its a directoy, will add this option later
        } else {
            // Creates a new path entry
            file_list.p = (paths*)realloc(file_list.p,
                sizeof(paths) * (file_list.size + 1));
            std::string filename;
            if (placeholder.size() > 20) {
                int slash = placeholder.find_last_of('/');
                filename = placeholder.substr(slash+1);
            } else {
                filename = placeholder;
            }
            file_list.p[file_list.size].filename =
                (char*)malloc(filename.size() + 1);
            memcpy(file_list.p[file_list.size].filename,
                filename.c_str(), filename.size());
            file_list.p[file_list.size].filename[filename.size()] = '\0';

            file_list.p[file_list.size].path = (char*)malloc(len + 1);
            memcpy(file_list.p[file_list.size].path, placeholder.c_str(), len);
            file_list.p[file_list.size].path[len] = '\0';
            file_list.p[file_list.size].size = len;
            file_list.size++;
        }
    }
}
/*Returns true if the path given is a directory*/
bool FileEditor::isDirectory(std::string path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            // it's a directory
            return true;
        } else if (s.st_mode & S_IFREG) {
            // it's a file
            return false;
        }
    }
    // Not a file or a directory
    return -1;
}

/*Checks if the path exists*/
bool FileEditor::doesPathExist() {
  struct stat buffer;
  return (stat (complete_path.c_str(), &buffer) == 0);
}

// Terminal handling
/*Exits raw mode at exit. Needs to be outside class to function.*/
void FileEditor::disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

/*Enables raw mode*/
void FileEditor::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/*Gets current size of the terminal window.*/
int FileEditor::getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*Append the char strings in row and then adds ~ to mark end of file.*/
void FileEditor::editorDrawRows() {
    int y;
    for (y = 0; y < screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == screenrows / 3) {
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
            } else {
                bufferAppend("~", 1);
            }
        } else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > screencols) len = screencols;
            bufferAppend(&E.row[filerow].render[E.coloff], len);
        }
        bufferAppend("\x1b[K", 3);
        bufferAppend("\r\n", 2);
    }
}

/*Appends the status bar with white background showing filename
and current_line/total_lines.*/
void FileEditor::editorDrawStatusBar() {
    bufferAppend("\x1b[7m", 4);
    char status[80];
    char rstatus[80];

    int len = snprintf(status, sizeof(status), "%.20s",
        file_list.p[file_number].filename ?
        file_list.p[file_number].filename : "[No Name]");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
        c.y +1, E.numrows);
    if (len > screencols) len = screencols;
    bufferAppend(status, len);
    while (len < screencols) {
        if (screencols - len == rlen) {
            bufferAppend(rstatus, rlen);
            break;
        } else {
            bufferAppend(" ", 1);
            len++;
        }
    }
    bufferAppend("\x1b[m", 3);
    bufferAppend("\r\n", 2);
}

/*Simply render message bar for displaying info.*/
void FileEditor::editorDrawMessageBar() {
    bufferAppend("\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > screencols) msglen = screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        bufferAppend(E.statusmsg, msglen);
}

/*Updates the row char string to identify and modify the display of tabs.*/
void FileEditor::editorUpdateRow(erow *row, int at) {
    int tabs = 0;
    // int at = E.numrows;
    int j;
    debug.send(row->chars);
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') tabs++;
    }

    free(E.row[at].render);
    row->render = (char*)malloc(row->size + tabs*(TAB_STOP - 1) + 1);
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
    E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));
    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = (char*)malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at], at);
    E.numrows++;
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

/*???*/
void FileEditor::editorSetStatusMessage(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

// Cursor handling
/*Runs if the normal way of retrieving the window size doesnt work.
Runs by placing the mouse as far as possible into the corner while
still visible to determine window size.*/
int FileEditor::getCursorPosition(int *rows, int *cols) {
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

/*Moves the cursor according to the input from editorReadKey.*/
void FileEditor::editorMoveCursor(int key) {
    /* Moves cursor based on cases it gets from */

    erow *row = (c.y >= E.numrows) ? NULL : &E.row[c.y];
    switch (key) {
        case ARROW_LEFT:
            if (c.x != 0) {
                c.x--;
            } else if (c.y > 0) {
                c.y--;
                c.x = E.row[c.y].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && c.x < row->size) {
                c.x++;
            } else if (row && c.x == row->size) {
                c.y++;
                c.x = 0;
            }
            break;
        case ARROW_UP:
            if (c.y != 0) c.y--;
            break;
        case ARROW_DOWN:
            if (c.y < E.numrows) c.y++;
            break;
    }
    row = (c.y >= E.numrows) ? NULL : &E.row[c.y];
    int rowlen = row ? row->size : 0;
    if (c.x > rowlen) c.x = rowlen;
}

/*Compensates the vertical position of the cursor since tabs take
up more than 1 character space.*/
int FileEditor::editorRowCxToRx() {
    int rx = 0;
    int j;
    for (j = 0; j < c.x; j++) {
        if (E.row[c.y].chars[j] == '\t')
        rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

// Insert handling

void FileEditor::editorRowInsertChar(int at, int input) {
  if (at < 0 || at > E.row[c.y].size) at = E.row[c.y].size;
  debug.send(at);
  debug.send(E.row[c.y].size);
  E.row[c.y].chars = (char*)realloc(E.row[c.y].chars, E.row[c.y].size + 2);
  debug.send((char*)"before mem move");
  memmove(&E.row[c.y].chars[at + 1], &E.row[c.y].chars[at], E.row[c.y].size - (at + 1));
  debug.send((char*)"after mem move");
  E.row[c.y].size++;
  E.row[c.y].chars[at] = input;
  debug.send((char*)"after reassign");
  editorUpdateRow(&E.row[c.y], c.y);
  debug.send((char*)"got here?");
}

void FileEditor::editorInsertChar(int read_key) {
    if (c.y == E.numrows) {
        editorAppendRow((char*)"", 0);
    }
    editorRowInsertChar(c.x, read_key);
    c.x++;
}

// Input handling
/*Opens a file taken as argument and send the content to the buffer.*/
void FileEditor::editorOpen(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                                line[linelen - 1] == '\r'))
            linelen--;
        editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
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

/*Frees all the rows generated from file*/
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

/*Detects new input and formats it into a int.*/
int FileEditor::editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    // Special case for arrow keys
    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    } else {
        return c;
    }
}

/*Processes the available inputs.*/
bool FileEditor::editorProcessKeypress() {
    int read_key = editorReadKey();
    switch (read_key) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            return false;
            break;
        // Experimental, needs to flush saved rows.
        case CTRL_KEY('w'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            if (file_number >= 0 && file_number < file_list.size) {
                file_number++;
                resetRows();
                editorOpen(file_list.p[file_number].path);
            }
            break;
        case HOME_KEY:
            c.x = 0;
            break;
        case END_KEY:
            {
                if (c.y < E.numrows)
                    c.x = E.row[c.y].size;
            }
            break;
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (read_key == PAGE_UP) {
                    c.y = E.rowoff;
                } else if (read_key == PAGE_DOWN) {
                    c.y = E.rowoff + screenrows - 1;
                    if (c.y > E.numrows) c.y = E.numrows;
                }

                int times = screenrows;
                while (times--)
                    editorMoveCursor(read_key == PAGE_UP ?
                        ARROW_UP : ARROW_DOWN);
            }
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(read_key);
            break;
        default:
            editorInsertChar(read_key);
            break;
    }

    return true;
}

/*Adds the default values and makes functions calls
the other info to be displayed.*/
void FileEditor::editorRefreshScreen() {
    editorScroll();

    bufferAppend("\x1b[?25l", 6);
    bufferAppend("\x1b[H", 3);

    editorDrawRows();
    editorDrawStatusBar();
    editorDrawMessageBar();

    char buf[32];
    // Tracks cursor
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (c.y - E.rowoff) + 1,
        (c.rx - E.coloff) + 1);
    bufferAppend(buf, strlen(buf));

    bufferAppend("\x1b[?25h", 6);

    write(STDOUT_FILENO, buffer.b, buffer.len);
    buffer.len = 0;
    buffer.b = (char*)realloc(buffer.b, 0);
}
