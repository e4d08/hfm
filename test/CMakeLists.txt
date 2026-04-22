add_mocked_test(compress_decompress
    SOURCES ${CMAKE_SOURCE_DIR}/src/hfm.c ${CMAKE_SOURCE_DIR}/src/huffman_tree.c 
#   MOCKS getc putc fprintf exit
#   COMPILE_OPTIONS -fno-builtin
)
