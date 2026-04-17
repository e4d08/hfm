cmake_minimum_required(VERSION 3.10)
project(hfm VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_CLANG_TIDY clang-tidy)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

add_subdirectory(src)
