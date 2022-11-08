#include "doctest/doctest.h"
#include "include/file_editor.hpp"
#include <fstream>

TEST_CASE("Loading of files") {
    std::string test_path = "../test_env/";
    std::string comparison = "Fivel\n\nother info\n";

    if (!fs::is_directory(test_path) || !fs::exists(test_path)) {
        fs::create_directory(test_path);
    }

    std::ofstream test_file;
    test_file.open(test_path + "file.txt");
    test_file << comparison;
    test_file.close();

    FileEditor test(test_path);
    test.runtime(false);

    int len = 5;
    const char* output = test.editorRowToString(&len);
    std::string output_str(output);

    CHECK(output_str == comparison);
    CHECK(output_str.size() == comparison.size());
}