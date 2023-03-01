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

    int len = 0;
    std::vector<std::string> buf = editorRowToString(len);
    std::ofstream file;
    file.open(file_list.p[file_number].path, std::ofstream::trunc);
    for (int i = 0; i < buf.size(); i++) {
        file << buf[i] << std::endl;
        E.dirty = 0;
        editorSetStatusMessage("%d characters written to disk", len);
    }
    file.close();
    return;
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

/*Opens a file taken as argument and send the content to the buffer.*/
void FileEditor::editorOpen(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp)
        die("fopen");

    char* line = NULL;
    std::string line_str = "";
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        line_str = line;
        while (line_str.size() > 0 && (line_str[line_str.size() - 1] == '\n' ||
                                       line_str[line_str.size() - 1] == '\r'))
            line_str.erase(line_str.size() - 1, 1);
        editorInsertRow(E.numrows, line_str, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}
