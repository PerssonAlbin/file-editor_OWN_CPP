#include "doctest/doctest.h"
#include "include/file_editor.hpp"

TEST_CASE("test buffer handling") {
    std::string test_path = "../test_env/";
    FileEditor test(test_path);
    test.runtime(false);

    int len = 5;
    const char* output = test.editorRowToString(&len);
    std::string output_str(output);

    std::string comparison = "Fivel\n";
    CHECK(output_str == comparison);
}