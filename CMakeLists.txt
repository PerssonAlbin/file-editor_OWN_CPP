cmake_minimum_required(VERSION 3.18)
project(file-editor.out)

set(CMAKE_CXX_STANDARD 17)
include_directories(.)
add_executable(file-editor.out src/main.cpp src/file_editor.cpp include/file_editor.hpp)