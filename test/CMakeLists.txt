add_library(unity STATIC third_party/unity/unity.c)
target_include_directories(unity PUBLIC third_party/unity)

add_executable(test_hfm
    unit/test_hfm.c
)
target_link_libraries(test_hfm PRIVATE hfm_lib unity)
