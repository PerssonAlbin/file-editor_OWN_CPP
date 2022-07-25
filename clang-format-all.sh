#!/bin/sh

find src include test\
  \( -name '*.c' \
  -o -name '*.cc' \
  -o -name '*.cpp' \
  -o -name '*.h' ! -name 'doctest.h' \
  -o -name '*.hh' \
  -o -name '*.hpp' \) \
  -exec clang-format -style=file -i --verbose {} \;
