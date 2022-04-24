// Copyright 2022 Albin Persson
#include <iostream>
#include "include/file_editor.hpp"

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Max 2 arguments allowed\n";
        return -1;
    }
    std::string dir_path = "";
    if (argc > 1) {
        dir_path = argv[1];
    }
    try {
        FileEditor editor(argc, dir_path);
        editor.runtime();
    }
    catch(std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
