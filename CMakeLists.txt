cmake_minimum_required(VERSION 3.10)
project(hfm VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

add_subdirectory(src)
