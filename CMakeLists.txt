cmake_minimum_required(VERSION 3.18)
project(file-editor.out)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDOCTEST_CONFIG_DISABLE")
set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(.)
add_executable(file-editor.out
    src/main.cpp
    src/file_editor/file_editor.cpp
    src/file_editor/buffer_handling.cpp
    src/file_editor/editor_handling.cpp
    src/file_editor/file_handling.cpp
    src/file_editor/terminal_handling.cpp
    src/syntax/syntax_highlight.cpp
    include/file_editor.hpp
    include/syntax_highlight.hpp
    src/file_type/file_type.cpp)
