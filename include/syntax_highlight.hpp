// Copyright 2022 Albin Persson
#ifndef INCLUDE_SYNTAX_HIGHLIGHT_HPP_
#define INCLUDE_SYNTAX_HIGHLIGHT_HPP_

#include <vector>
#include <iostream>
#include <regex>
#include <string.h>

class SyntaxHighlight {
 private:
   std::string cppStyle(char* line);
   int detectFiletype(char* filename);
 public:
   std::string hightlightLine(char* line, char* filename);
};


#endif  // INCLUDE_SYNTAX_HIGHLIGHT_HPP_
