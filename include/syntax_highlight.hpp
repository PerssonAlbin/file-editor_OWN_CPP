// Copyright 2022 Albin Persson
#ifndef INCLUDE_SYNTAX_HIGHLIGHT_HPP_
#define INCLUDE_SYNTAX_HIGHLIGHT_HPP_

#include <vector>
#include <iostream>
#include <regex>
#include <string.h>

class SyntaxHighlight {
 private:
   std::string cpp_style(char* line);
   int detect_filetype(char* filename);
 public:
   std::string hightlight_line(char* line, char* filename);
};


#endif  // INCLUDE_SYNTAX_HIGHLIGHT_HPP_
