// Copyright 2022 Albin Persson
#include "include/file_editor.hpp"

std::string FileEditor::wstringToString(std::wstring wstring_to_conv) {
    std::string string_to_return;

    std::transform(wstring_to_conv.begin(), wstring_to_conv.end(),
                   std::back_inserter(string_to_return),
                   [](wchar_t c) { return (char)c; });
    return string_to_return;
}