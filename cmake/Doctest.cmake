# Add doctest to the build if needed
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DENABLE_DOCTEST_IN_LIBRARY)
    include(FetchContent)
    FetchContent_Declare(
            doctest
            GIT_REPOSITORY "https://github.com/doctest/doctest"
            GIT_TAG "v2.4.9"
    )

    FetchContent_MakeAvailable(DocTest)
    include_directories(${DOCTEST_INCLUDE_DIR})
endif()