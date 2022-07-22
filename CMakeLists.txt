cmake_minimum_required(VERSION 3.18)
project(file-editor.out)

option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Doctest)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDOCTEST_CONFIG_DISABLE -DENABLE_DOCTESTS=OFF")
set(EXECUTABLE_OUTPUT_PATH ../bin/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/file_editor/file_editor.cpp
    src/file_editor/buffer_handling.cpp
    src/file_editor/editor_handling.cpp
    src/file_editor/file_handling.cpp
    src/file_editor/terminal_handling.cpp
    src/syntax/syntax_highlight.cpp
    src/file_type/file_type.cpp)

set(TESTFILES
    tests/main.cpp)

set(LIBRARY_NAME editor)

# Compile all sources into a library.
add_library(${LIBRARY_NAME} OBJECT ${SOURCES})

# Add header files to compilation
include_directories(.)

# Link doctest if it is used to source library
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(${LIBRARY_NAME} PUBLIC doctest)
endif()

add_executable(file-editor.out src/main.cpp)
target_link_libraries(file-editor.out PRIVATE ${LIBRARY_NAME})

# Set up tests (see tests/CMakeLists.txt).
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(tests)
endif()