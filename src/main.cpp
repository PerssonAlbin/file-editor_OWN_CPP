// Copyright 2022 Albin Persson
#include <iostream>
#include "include/file_editor.hpp"

/*Fixes the path taken as argument when starting the program.*/
std::string formatPath(int argc, std::string argv) {
    char cwd[256];
    std::string complete_path;
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
    return complete_path;
}

int main(int argc, char* argv[]) {
    std::string dir_path = "";
    if (argc > 2) {
        std::cout << "Max 2 arguments allowed\n";
        return -1;
    } else if (argc == 2) {
        dir_path = formatPath(argc, argv[1]);
    } else {
        dir_path = formatPath(argc, "");
    }
    try {
        FileEditor editor(dir_path);
        editor.runtime();
    }
    catch(std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
