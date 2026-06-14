cmake_minimum_required(VERSION 3.14)
project(hfm VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
    
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_C_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
    add_compile_options(/W4)
endif()

option(ENABLE_COVERAGE "enable coverage" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

add_compile_options(-Wall -Wextra -pedantic -Wshadow -Wconversion -Werror)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
