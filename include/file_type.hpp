// Copyright 2022 Andreas Bauer

#ifndef INCLUDE_FILE_TYPE_HPP_
#define INCLUDE_FILE_TYPE_HPP_

#include <algorithm>
#include <iostream>
#include <string>

namespace filetype {
    enum class FileType { txt, md, cpp };

    FileType fileTypeFromStr(const std::string val);
    std::string fileTypeToStr(FileType type);
} // namespace filetype

#endif // INCLUDE_FILE_TYPE_HPP_
