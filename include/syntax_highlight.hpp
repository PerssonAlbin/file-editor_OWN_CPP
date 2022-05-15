// Copyright 2022 Albin Persson
#ifndef INCLUDE_SYNTAX_HIGHLIGHT_HPP_
#define INCLUDE_SYNTAX_HIGHLIGHT_HPP_

#include <vector>
#include <iostream>
#include <regex>
#include <string.h>


const char* SYNTAX_YELLOW = "\x1b[33m";
const char* SYNTAX_BLUE = "\x1b[36m";
const char* SYNTAX_GREEN = "\x1b[92m";
const char* SYNTAX_BRIGHT_YELLOW = "\x1b[93m";
const char* SYNTAX_RESET = "\x1b[0m";

class SyntaxHighlight {
 private:
   std::string cpp_style(char* line);
   int detect_filetype(char* filename);
 public:
   std::string hightlight_line(char* line, char* filename);
};


#endif  // INCLUDE_SYNTAX_HIGHLIGHT_HPP_
