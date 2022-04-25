// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

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
            file_list.p = reinterpret_cast<paths*>(
                realloc(file_list.p, sizeof(paths) * (file_list.size + 1)));
            std::string filename;
            if (placeholder.size() > 20) {
                int slash = placeholder.find_last_of('/');
                filename = placeholder.substr(slash+1);
            } else {
                filename = placeholder;
            }
            file_list.p[file_list.size].filename = reinterpret_cast<char*>(
                malloc(filename.size() + 1));
            memcpy(file_list.p[file_list.size].filename,
                filename.c_str(), filename.size());
            file_list.p[file_list.size].filename[filename.size()] = '\0';

            file_list.p[file_list.size].path = reinterpret_cast<char*>(
                malloc(len + 1));
            memcpy(file_list.p[file_list.size].path, placeholder.c_str(), len);
            file_list.p[file_list.size].path[len] = '\0';
            file_list.p[file_list.size].size = len;
            file_list.size++;
        }
    }
}

/* Returns true if the path given is a directory */
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
bool FileEditor::pathExist() {
  struct stat buffer;
  return (stat (complete_path.c_str(), &buffer) == 0);
}

void FileEditor::editorSave() {
    if (file_list.p[file_number].path == NULL) return;

    int len;
    char* buf = editorRowToString(&len);
    int fd = open(file_list.p[file_number].path, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                editorSetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

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
