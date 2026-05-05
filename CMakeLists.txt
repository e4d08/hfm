cmake_minimum_required(VERSION 3.14)
project(hfm VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  cmocka
  GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
  GIT_TAG        cmocka-1.1.5
  GIT_SHALLOW    1
)
FetchContent_MakeAvailable(cmocka)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
