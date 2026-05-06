add_compile_options(-Wall -Wextra -pedantic -Wshadow -Wconversion -Werror)

add_library(hfm_lib STATIC
    hfm/hfm.c
    hfm/huffman_tree.c
)
target_include_directories(hfm_lib PRIVATE hfm)

add_executable(hfm
    main.c
)
target_link_libraries(hfm PRIVATE hfm_lib)
