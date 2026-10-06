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
add_compile_options(-Wall -Wextra)

# Applies sanitisers to unit tests
# (shim must be free of sanitizer runtimes,
# which must load first and interpose pthreads themselves)
option(HEDDLE_SANITIZE "Build unit tests with ASan and UBSan" ON)
set(HEDDLE_SANITIZER_FLAGS
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer)

find_package(Threads REQUIRED)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
