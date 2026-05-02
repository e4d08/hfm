add_library(hfm_lib STATIC
    "${CMAKE_SOURCE_DIR}/src/hfm.c"
    "${CMAKE_SOURCE_DIR}/src/huffman_tree.c"
)
target_include_directories(hfm_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_executable(hfm
    main.c
)
target_link_libraries(hfm PRIVATE hfm_lib)
target_compile_options(hfm PRIVATE -Wall -Wextra -pedantic -Wshadow -Wconversion -Werror)
