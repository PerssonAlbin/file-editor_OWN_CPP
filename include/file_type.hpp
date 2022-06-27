// Copyright 2022 Andreas Bauer

#ifndef INCLUDE_FILE_TYPE_HPP_
#define INCLUDE_FILE_TYPE_HPP_

#include <iostream>
#include <string>
#include <algorithm>

namespace filetype {
    enum class FileType { txt, md, cpp };

    FileType fileTypeFromStr(const std::string val);
    std::string fileTypeToStr(FileType type);
}

#endif  // INCLUDE_FILE_TYPE_HPP_
