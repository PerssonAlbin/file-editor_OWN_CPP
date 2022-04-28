# file-editor_OWN_CPP
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CMake build](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/cmake.yml/badge.svg)](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/cmake.yml)
[![Lint Code](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/linter.yml/badge.svg)](https://github.com/DenmarkPolice/file-editor_OWN_CPP/actions/workflows/linter.yml)

## Table of contents

* [General info](#general-info)
* [Prerequisites](#Prerequisites)
* [Build](#build)
* [Commands](#Commands)
* [License](#License)

## General info
Simple command-line editor. Takes a directory as an argument and opens the files in that folder. Now have all the basic functionalities of a file editor.

## Prerequisites

Make sure you have installed all of the following prerequisites on your development machine:

* `g++` to compile the code
* `cmake` as build tool

You can install the prerequisites as follows:

On Ubuntu/Debian: `sudo apt install cmake g++ make`

On Fedora/CentOS: `sudo dnf install cmake gcc-c++ make`

On macOS: `brew install cmake`

## Build

To create a debug build run:

```shell
cmake .
make
```

To create a release build run:

```shell
cmake -DCMAKE_BUILD_TYPE=Release .
make
```

This will create an executable binary in the `bin/` folder.

## Commands

| Commands     ||
|--------------|-----------|
| Exit program | Ctrl-Q    |
| Switch file  | Ctrl-W    |

## License

Copyright (c) 2022 Albin Persson

This work (source code) is licensed under the [GNU General Public License v3.0](./LICENSE).
