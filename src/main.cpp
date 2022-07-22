// Copyright 2022 Albin Persson

#include "include/file_editor.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // doctest::Context context;
    // context.applyCommandLine(argc, argv);
    // Don't break in the debugger when assertions fail
    // context.setOption("no-breaks", true);

    // int res = context.run(); // run doctest

    // important - query flags (and --exit) rely on the user doing this
    // if (context.shouldExit()) {
    // propagate the result of the tests
    //     return res;
    // }

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " [path to dir]" << std::endl;
        return -1;
    }

    try {
        std::string dir_path = std::filesystem::canonical(argv[1]);
        FileEditor editor(dir_path);
        editor.runtime();
    } catch (std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Unable to resolve the given directory path.\n"
                  << e.what() << std::endl;
        return -1;
    }
    return 0;
}
