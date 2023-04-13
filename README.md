# file-editor_OWN_CPP
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CMake build](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/cmake.yml/badge.svg)](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/cmake.yml)
[![Lint Code](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/linter.yml/badge.svg)](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/linter.yml)

## Table of contents

- [file-editor\_OWN\_CPP](#file-editor_own_cpp)
  - [Table of contents](#table-of-contents)
  - [General info](#general-info)
  - [Prerequisites](#prerequisites)
  - [Build](#build)
  - [Commands](#commands)
  - [License](#license)

## General info
Simple command-line editor. Takes a directory as an argument and opens the files in that folder. Now have all the basic functionalities of a file editor.

## Prerequisites

Make sure you have installed all of the following prerequisites on your development machine:

* `g++` with minimum version `5.0` to compile the code
* `cmake` with minimum version `3.14` as build tool

You can install the prerequisites as follows:

##### On Ubuntu/Debian: 
Run: `sudo apt install g++ make`
And download the latest version of CMake from [here](https://cmake.org/download/)

##### On Fedora/CentOS: 
Run: `sudo dnf install gcc-c++ make`
And download the latest version of CMake from [here](https://cmake.org/download/)

##### On macOS: 
Run: `brew install cmake`

## Build

To unit test and debug build run:

```shell
cmake -DCMAKE_BUILD_TYPE=Debug . -B./build
cd build
make

# Run debug build
./file_editor.out

# Run unit tests
./unit_tests
```

To create a release build run:

```shell
cmake -DCMAKE_BUILD_TYPE=Release . -B./build
cd build
make
./file_editor.out
```

This will create an executable binary in the `build/` folder.

If you are using Visual Studio Code then we recommend the [CMake Tools Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) to build, debug, and run the application.

## Commands

| Commands     |        |
| ------------ | ------ |
| Exit program | Ctrl-Q |
| Switch file  | Ctrl-W |
| Save file    | Ctrl-S |

## License

Copyright (c) 2022 Albin Persson

This work (source code) is licensed under the [GNU General Public License v3.0](./LICENSE).
