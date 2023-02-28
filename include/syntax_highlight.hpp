// Copyright 2022 Albin Persson
#ifndef INCLUDE_SYNTAX_HIGHLIGHT_HPP_
#define INCLUDE_SYNTAX_HIGHLIGHT_HPP_

#include <iostream>
#include <regex> // NOLINT
#include <string.h>
#include <string>
#include <vector>

class SyntaxHighlight {
  private:
    int comment_index = -1;
    std::string cppStyle(std::string line);
    int detectFiletype(char* filename);
    std::string injectColor(std::string search_text, std::regex regex_check);

  public:
    unsigned int added_length = 0;
    std::string hightlightLine(std::string line, char* filename);
};

#endif // INCLUDE_SYNTAX_HIGHLIGHT_HPP_
