// Copyright 2022 Albin Persson
#ifndef INCLUDE_SYNTAX_HIGHLIGHT_HPP_
#define INCLUDE_SYNTAX_HIGHLIGHT_HPP_

#include <string.h>
#include <vector>
#include <iostream>
#include <regex>
#include <string>

class SyntaxHighlight {
 private:
    int comment_index = -1;
    std::string cppStyle(char* line);
    int detectFiletype(char* filename);
    std::string injectColor(std::string search_text, std::regex regex_check);
 public:
    unsigned int added_length = 0;
    std::string hightlightLine(char* line, char* filename);
};

#endif  // INCLUDE_SYNTAX_HIGHLIGHT_HPP_
