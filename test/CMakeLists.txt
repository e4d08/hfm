add_library(unity STATIC third_party/unity/unity.c)
target_include_directories(unity PUBLIC third_party/unity)

add_executable(test_hfm unit/test_hfm.c)
target_link_libraries(test_hfm PRIVATE hfm_lib unity)

add_executable(test_block unit/test_block.c)
target_link_libraries(test_block PRIVATE hfm_lib unity)

include(CTest)
add_test(NAME test_hfm COMMAND test_hfm)
add_test(NAME test_block COMMAND test_block)
