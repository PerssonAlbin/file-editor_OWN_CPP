// Copyright 2022 Andreas Bauer
#include "test/doctest.h"

#include "include/file_type.hpp"

namespace filetype {

    std::string fileTypeToStr(FileType type) {
        switch (type) {
        case FileType::cpp:
            return "C/C++";
        case FileType::md:
            return "Markdown";
        case FileType::txt:
        default:
            return "Textfile";
        }
    }

    FileType fileTypeFromStr(const std::string file_ext) {
        std::string lower_ext(file_ext);
        std::for_each(lower_ext.begin(), lower_ext.end(),
                      [](char& c) { c = ::tolower(c); });

        auto isCPP = "cpp" == lower_ext || "hpp" == lower_ext ||
                     "c" == lower_ext || "cc" == lower_ext || "h" == lower_ext;
        if (isCPP) {
            return FileType::cpp;
        }

        auto isMarkdown = "md" == lower_ext || "markdown" == lower_ext ||
                          "mdown" == lower_ext || "mkdn" == lower_ext ||
                          "mkd" == lower_ext || "rmd" == lower_ext;
        if (isMarkdown) {
            return FileType::md;
        }

        return FileType::txt;
    }

    TEST_CASE("should return correct FileType") {
        CHECK(fileTypeFromStr("") == FileType::txt);
        CHECK(fileTypeFromStr("UNKNOWN") == FileType::txt);

        CHECK(fileTypeFromStr("c") == FileType::cpp);
        CHECK(fileTypeFromStr("cc") == FileType::cpp);
        CHECK(fileTypeFromStr("cpp") == FileType::cpp);
        CHECK(fileTypeFromStr("h") == FileType::cpp);
        CHECK(fileTypeFromStr("hpp") == FileType::cpp);

        CHECK(fileTypeFromStr("md") == FileType::md);
        CHECK(fileTypeFromStr("mkdn") == FileType::md);
        CHECK(fileTypeFromStr("mdown") == FileType::md);
        CHECK(fileTypeFromStr("markdown") == FileType::md);
        CHECK(fileTypeFromStr("mkd") == FileType::md);
        CHECK(fileTypeFromStr("rmd") == FileType::md);

        CHECK(fileTypeFromStr("CPP") == FileType::cpp);
        CHECK(fileTypeFromStr("cPp") == FileType::cpp);
    }

} // namespace filetype
