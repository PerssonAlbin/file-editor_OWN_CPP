#include "include/syntax_highlight.hpp"

const char* SYNTAX_YELLOW = "\x1b[33m";
const char* SYNTAX_BLUE = "\x1b[36m";
const char* SYNTAX_GREEN = "\x1b[92m";
const char* SYNTAX_BRIGHT_YELLOW = "\x1b[93m";
const char* SYNTAX_RESET = "\x1b[0m";

int SyntaxHighlight::detect_filetype(char* file_ext) {
    if (strcmp(file_ext, ".cpp") ||
        strcmp(file_ext, ".c") ||
        strcmp(file_ext, ".hpp") ||
        strcmp(file_ext, ".h")) {
            return 1;
        }
    return 0;
}

std::string SyntaxHighlight::hightlight_line(char* line, char* file_ext) {
    int c = detect_filetype(file_ext);

    switch(c) {
        // C/C++ code
        case 1:
            return cpp_style(line);
            break;
        // No case found
        default:
            // Should set status message of the editor to no highlighting found
            return line;
            break;
    }
}

std::string SyntaxHighlight::cpp_style(char* line) {
    int last_found = 0;
    int current = 0;
    int line_size = 0;

    //Regex combinations
    auto const regex_string = std::regex("\".*\"");
    auto const regex_comment = std::regex("(//).*");
    auto const regex_statement = std::regex(
        "(std::endl|std::cerr|std::ifstream|std::cout|break|if|while|switch|case|throw|catch|default|do|goto|return|try|for|#include)");
    auto const regex_type = std::regex(
        "(|int|std::string|std::vector|long|char|size_t|namespace)");
    auto searchResults = std::smatch{};
    std::string search_text = std::string(line);

    // Comments
    if (std::regex_search(search_text, searchResults, regex_comment)) {
        std::vector<std::pair<int,int>> index_length;
        for (auto it = std::sregex_iterator(search_text.begin(), search_text.end(), regex_comment);
        it != std::sregex_iterator();
        ++it) {
            index_length.push_back(std::make_pair<int, int>(it->position(), it->length()));
        }
        search_text.insert(index_length[0].first+index_length[0].second, SYNTAX_RESET);
        search_text.insert(index_length[0].first, SYNTAX_GREEN);
    }

    // Strings
    if (std::regex_search(search_text, searchResults, regex_string)) {
        std::vector<std::pair<int,int>> index_length;
        for (auto it = std::sregex_iterator(search_text.begin(), search_text.end(), regex_string);
        it != std::sregex_iterator();
        ++it) {
            index_length.push_back(std::make_pair<int, int>(it->position(), it->length()));
        }
        search_text.insert(index_length[0].first+index_length[0].second, SYNTAX_RESET);
        search_text.insert(index_length[0].first, SYNTAX_BRIGHT_YELLOW);
    }
    // Types
    if (std::regex_search(search_text, searchResults, regex_type)) {
        std::vector<std::pair<int,int>> index_length;
        for (auto it = std::sregex_iterator(search_text.begin(), search_text.end(), regex_type);
        it != std::sregex_iterator();
        ++it) {
            index_length.push_back(std::make_pair<int, int>(it->position(), it->length()));
        }
        search_text.insert(index_length[0].first+index_length[0].second, SYNTAX_RESET);
        search_text.insert(index_length[0].first, SYNTAX_BLUE);
    }

    // Statements
    if (std::regex_search(search_text, searchResults, regex_statement)) {
        std::vector<std::pair<int,int>> index_length;
        for (auto it = std::sregex_iterator(search_text.begin(), search_text.end(), regex_statement);
        it != std::sregex_iterator();
        ++it) {
            index_length.push_back(std::make_pair<int, int>(it->position(), it->length()));
        }
        search_text.insert(index_length[0].first+index_length[0].second, SYNTAX_RESET);
        search_text.insert(index_length[0].first, SYNTAX_YELLOW);
    }
    return (search_text);
}
