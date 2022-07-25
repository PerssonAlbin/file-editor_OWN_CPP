#include "include/file_type.hpp"
#include "test/doctest.h"

namespace filetype {

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
