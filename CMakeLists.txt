cmake_minimum_required(VERSION 3.22)
project(heddle LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)
enable_testing()

add_subdirectory(tools/tests)