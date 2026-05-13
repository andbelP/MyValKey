cmake_minimum_required(VERSION 3.10)

project(keyval)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)



add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
