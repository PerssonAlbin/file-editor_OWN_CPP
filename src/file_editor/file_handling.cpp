// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

namespace fs = std::filesystem;

/*Iterates through the current path until all files and
directories have been added to file_list.*/
void FileEditor::createFileList() {
    std::string placeholder;
    int len;
    auto dir_iter = fs::recursive_directory_iterator(this->complete_path);
    for (const auto& entry : dir_iter) {
        placeholder = entry.path();
        len = placeholder.size();
        if (fs::is_directory(placeholder)) {
            // TODO(PerssonAlbin): Implement directory handling
            continue;
        }

        paths path;

        std::string filename = this->trimFilename(placeholder, 20);

        path.filename = reinterpret_cast<char*>(malloc(filename.size() + 1));
        memcpy(path.filename, filename.c_str(), filename.size());
        path.filename[filename.size()] = '\0';

        path.path = reinterpret_cast<char*>(malloc(len + 1));
        memcpy(path.path, placeholder.c_str(), len);
        path.path[len] = '\0';
        path.size = len;

        auto file_extension = filename.substr(filename.find_last_of(".") + 1);
        auto filetype = ft::fileTypeFromStr(file_extension);
        path.filetype = filetype;

        file_list.p = reinterpret_cast<paths*>(
            realloc(file_list.p, sizeof(paths) * (file_list.size + 1)));

        file_list.p[file_list.size] = path;
        file_list.size++;
    }
}

std::string FileEditor::trimFilename(std::string filename, int length) {
    if (filename.size() < length) {
        return filename;
    }

    int slash_idx = filename.find_last_of('/');
    return filename.substr(slash_idx + 1);
}

void FileEditor::editorSave() {
    if (file_list.p[file_number].path == NULL) {
        return;
    }

    int len;
    char* buf = editorRowToString(&len);
    int fd = open(file_list.p[file_number].path, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                E.dirty = 0;
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
void FileEditor::editorOpen(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp)
        die("fopen");

    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 &&
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editorInsertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}
