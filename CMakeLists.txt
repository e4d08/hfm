cmake_minimum_required(VERSION 3.14)
project(hfm VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_COVERAGE "enable coverage" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

add_compile_options(-Wall -Wextra -pedantic -Wshadow -Wconversion -Werror)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
