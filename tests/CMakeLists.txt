include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    EngineTests
    DBEngineTests.cpp
    GlobMatcherTests.cpp
)

target_link_libraries(
    EngineTests
    keyval
    GTest::gtest_main
)

target_include_directories(EngineTests PRIVATE ${PROJECT_SOURCE_DIR}/src)

include(GoogleTest)

gtest_discover_tests(EngineTests)