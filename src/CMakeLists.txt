add_executable(hfm
    hfm.c
    huffman_tree.c
    main.c
)

target_compile_options(hfm PRIVATE -Wall -Wextra -pedantic -Wshadow -Wconversion -Werror)
