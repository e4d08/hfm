add_library(unity STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/unity/unity.c
)
target_include_directories(unity PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/unity
)

add_executable(test_hfm
    test_hfm.c
)
target_link_libraries(test_hfm PRIVATE hfm_lib unity)
