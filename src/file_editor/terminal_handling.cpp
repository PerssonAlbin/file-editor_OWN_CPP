// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

void FileEditor::disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

void FileEditor::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
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
int FileEditor::getWindowSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Moves cursor
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return -1;
        }
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
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

/*Runs if the normal way of retrieving the window size doesnt work.
Runs by placing the mouse as far as possible into the corner while
still visible to determine window size.*/
int FileEditor::getCursorPosition(int* rows, int* cols) {
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        return -1;
    }
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        }
        if (buf[i] == 'R') {
            break;
        }
        i++;
    }
    buf[i] = END_OF_ROW;
    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    }
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }
    return 0;
}

/*Detects new input and formats it into a int.*/
int FileEditor::editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }
    // Special case for arrow keys
    if (c == TERM_ESC) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return TERM_ESC;
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return TERM_ESC;
        }
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return TERM_ESC;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
            }
        }
        return TERM_ESC;
    } else {
        return c;
    }
}

/*Processes the available inputs.*/
bool FileEditor::editorProcessKeypress() {
    static int quit_times = QUIT_TIMES;
    int read_key = editorReadKey();
    switch (read_key) {
    case '\r':
        editorInsertNewline();
        break;
    case CTRL_KEY('q'):
        if (E.dirty && quit_times > 0) {
            editorSetStatusMessage(
                "File has unsaved changes. Press Ctrl-Q again to quit");
            quit_times--;
            return true;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        return false;
        break;
    case CTRL_KEY('s'):
        editorSave();
        break;
    // Experimental, needs to skip executables.
    case CTRL_KEY('w'): {
        write(STDOUT_FILENO, TERM_CLEAR_SCREEN, 4);
        write(STDOUT_FILENO, TERM_SEND_CURSOR_HOME, 3);
        if (file_number >= file_list.size - 1) {
            file_number = 0;
        } else {
            file_number++;
        }
        resetRows();
        char* test_val = file_list.p[file_number].path;
        editorOpen(file_list.p[file_number].path);
    } break;
    case HOME_KEY:
        c.x = 0;
        break;
    case END_KEY: {
        if (c.y < E.numrows)
            c.x = E.rows[c.y].chars.size();
    } break;
    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
        if (read_key == DEL_KEY)
            editorMoveCursor(ARROW_RIGHT);
        editorDelChar();
        break;
    case PAGE_UP:
    case PAGE_DOWN: {
        if (read_key == PAGE_UP) {
            c.y = E.rowoff;
        } else if (read_key == PAGE_DOWN) {
            c.y = E.rowoff + screenrows - 1;
            if (c.y > E.numrows)
                c.y = E.numrows;
        }

        int times = screenrows;
        while (times--)
            editorMoveCursor(read_key == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    } break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editorMoveCursor(read_key);
        break;
    case CTRL_KEY('l'):
    case TERM_ESC:
        break;
    default: {
        if (read_key == -61) {
            int second_read_key = editorReadKey();
            read_key = (read_key * -1) - 1;
            second_read_key = (second_read_key * -1) - 1;
            std::string str_key = decimalToHex(read_key);
            std::string str_second_key = decimalToHex(second_read_key);
            std::string hex_str = "     ";
            hex_str[0] = str_key[1];
            hex_str[1] = str_key[0];
            hex_str[3] = str_second_key[1];
            hex_str[4] = str_second_key[0];

            // Convert hex string to integer
            std::stringstream ss;
            ss << std::hex << hex_str;
            unsigned int hex_int;
            ss >> hex_int;

            wchar_t wc = std::strtol(hex_str.c_str(), NULL, 16);
            std::wstring test = L"å";
            int len = test.length();
            // std::string attempt_conv = codepoint_to_utf8(hex_str.c_str());
            //  Output the UTF-8 encoded string
            // std::wcout << wc << std::endl;

                        // Convert to Unicode hex
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring unicode_hex = converter.from_bytes(hex_str);
            std::wcout << std::hex << std::uppercase
                       << static_cast<int>(unicode_hex[0]) << std::endl;

            std::cout << "xxx" << std::endl;
        }
        editorInsertChar(read_key);

    } break;
    }
    quit_times = QUIT_TIMES;
    return true;
}

std::string FileEditor::decimalToHex(int decimalValue) {
    std::stringstream stream;
    stream << std::hex << decimalValue; // convert to hexadecimal string
    return stream.str();                // return the hexadecimal string
}

std::string FileEditor::getUnicodeString(const std::string& hex) {
    std::istringstream codepoints{hex};
    std::string cp;
    std::string out;
    std::mbstate_t state;
    char u8[MB_LEN_MAX];

    while (codepoints >> cp) {
        char32_t c = std::stoul(cp, nullptr, 16);
        auto len = std::c32rtomb(u8, c, &state);
        if (len == std::size_t(-1)) {
            std::cerr << "Unable to convert " << cp << " to UTF-8 codepoint!\n";
            std::exit(EXIT_FAILURE);
        } else if (len > 0) {
            out.append(u8, len);
        }
    }
    return out;
}

std::string FileEditor::codepoint_to_utf8(char32_t codepoint) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.to_bytes(&codepoint, &codepoint + 1);
}
