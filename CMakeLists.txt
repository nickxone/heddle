cmake_minimum_required(VERSION 3.22)
project(heddle LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif ()

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "heddle builds on Linux only (LD_PRELOAD, ELF). "
            "Use the Docker toolchain or ./docker/run.sh.")
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra)

option(HEDDLE_CLANG_TIDY "Run clang-tidy on C++ sources while building" OFF)
if (HEDDLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
endif ()

# Applies sanitisers to unit tests
# (shim must be free of sanitizer runtimes,
# which must load first and interpose pthreads themselves)
option(HEDDLE_SANITIZE "Build unit tests with ASan and UBSan" ON)
set(HEDDLE_SANITIZER_FLAGS
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer)

find_package(Threads REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
