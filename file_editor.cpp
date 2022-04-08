#include "file_editor.hpp"

#include "debug/send_debug.cpp"

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#define CTRL_KEY(k) ((k) & 0x1f)
#define EDITOR_VERSION "0.0.1"
namespace fs = std::filesystem;

struct termios orig_termios;

/* Error handling */
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

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
    free(buffer.b);
}


/* Main function */
void FileEditor::runtime()
{
    enableRawMode();
    if(getWindowSize(&screenrows, &screencols) == -1) die("getWindowSize");
    
    //debug(file_list.p[file].path);
    editorOpen(file_list.p[file_number].path);
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
    std::string placeholder;
    int len;
    for (const auto & entry : fs::recursive_directory_iterator(this->path))
    {
        file_list.p = (paths*)realloc(file_list.p, sizeof(paths) * (file_list.size + 1));

        placeholder = entry.path();
        len = placeholder.size();
        file_list.p[file_list.size].path = (char*)malloc(len + 1);
        memcpy(file_list.p[file_list.size].path, placeholder.c_str(), len);
        file_list.p[file_list.size].path[len] = '\0';
        file_list.size++;
    }
}

bool FileEditor::doesPathExist()
{
  struct stat buffer;
  return (stat (path.c_str(), &buffer) == 0);
}

/* Terminal handling */

void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void FileEditor::enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int FileEditor::getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) 
    {
        if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void FileEditor::editorDrawRows()
{
    int y;
    for (y = 0; y < screenrows; y++)
    {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows)
        {
            if(E.numrows == 0 && y == screenrows / 3)
            {
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
        }
        else
        {
            int len = E.row[filerow].size - E.coloff;
            if (len < 0) len = 0;
            if (len > screencols) len = screencols;
            bufferAppend(&E.row[filerow].chars[E.coloff], len);
        }
        bufferAppend("\x1b[K", 3);
        if (y < screenrows - 1) {
            bufferAppend("\r\n", 2);
        }
    }
}

void FileEditor::editorAppendRow(char *s, size_t len)
{
    E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));
    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = (char*)malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}

void FileEditor::editorScroll()
{
    //Vertical scrolling
    if(c.y < E.rowoff)
    {
        E.rowoff = c.y;
    }
    if(c.y >= E.rowoff + screenrows)
    {
        E.rowoff = c.y - screenrows + 1;
    }
    if(c.x < E.coloff)
    {
        E.coloff = c.x;
    }
    if(c.x >= E.coloff + screencols)
    {
        E.coloff = c.x - screencols + 1;
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
    /* Moves cursor based on cases it gets from */

    erow *row = (c.y >= E.numrows) ? NULL : &E.row[c.y];
    switch (key)
    {
        case ARROW_LEFT:
            if(c.x != 0)
            {
                c.x--;
            }
            else if (c.y > 0)
            {
                c.y--;
                c.x = E.row[c.y].size;
            }
            break;
        case ARROW_RIGHT:
            if(row && c.x < row->size)
            {
                c.x++;
            }
            // Not sure I want this part.
            else if(row && c.x == row->size)
            {
                c.y++;
                c.x = 0;
            }
            break;
        case ARROW_UP:
            if(c.y != 0) c.y--;
            break;
        case ARROW_DOWN:
            if(c.y < E.numrows) c.y++;
            break;
    }
    row = (c.y >= E.numrows) ? NULL : &E.row[c.y];
    int rowlen = row ? row->size : 0;
    if (c.x > rowlen) c.x = rowlen;
}

/* Input handling */

void FileEditor::editorOpen(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    linelen = getline(&line, &linecap, fp);
    while ((linelen = getline(&line, &linecap, fp)) != -1)
    {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                                line[linelen - 1] == '\r'))
            linelen--;
        
        editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
}

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
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch(seq[1])
            {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
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
    int read_key = editorReadKey();
    switch (read_key)
    {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case CTRL_KEY('w'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            debug.send("Detect arrow click");
            if(file_number >= 0 && file_number < file_list.size)
            {
                
                file_number++;
                debug.send(file_list.p[file_number].path);
                editorOpen(file_list.p[file_number].path);
            }
            break;
        case HOME_KEY:
            c.x = 0;
            break;
        case END_KEY:
            c.x = screencols - 1;
            break;
        case PAGE_UP:
        case PAGE_DOWN:
            {
            int times = screenrows;
            while (times--)
                editorMoveCursor(read_key == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(read_key);
            break;
    }
    
}

void FileEditor::editorRefreshScreen()
{
    editorScroll();

    bufferAppend("\x1b[?25l", 6);
    bufferAppend("\x1b[H", 3);

    editorDrawRows();

    char buf[32];
    //Tracks cursor
      snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (c.y - E.rowoff) + 1, (c.x - E.coloff) + 1);
    bufferAppend(buf, strlen(buf));

    bufferAppend("\x1b[?25h", 6);

    write(STDOUT_FILENO, buffer.b, buffer.len);
    buffer.len = 0;
    buffer.b = (char*)realloc(buffer.b, 0);
}
