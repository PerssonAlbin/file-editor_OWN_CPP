#!/bin/sh

find src include \
  \( -name '*.c' \
  -o -name '*.cc' \
  -o -name '*.cpp' \
  -o -name '*.h' \
  -o -name '*.hh' \
  -o -name '*.hpp' \) \
  -exec clang-format -style=file -i --verbose {} \;
