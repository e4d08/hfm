include(CTest)

# юнит-тесты
add_library(unity STATIC third_party/unity/unity.c)
target_include_directories(unity PUBLIC third_party/unity)

add_executable(test_hfm unit/test_hfm.c)
target_link_libraries(test_hfm PRIVATE hfm_lib unity)

add_executable(test_block unit/test_block.c)
target_link_libraries(test_block PRIVATE hfm_lib unity)

add_test(NAME test_hfm COMMAND test_hfm)
add_test(NAME test_block COMMAND test_block)

# интеграционные тесты
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(GLOB TEST_FILES ${TEST_DATA_DIR}/*)

foreach(ORIG_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${ORIG_FILE} NAME)
    set(TEST_WORK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_test)
    set(COMPRESSED_FILE ${TEST_WORK_DIR}/${TEST_NAME}.hfm)
    set(DECOMPRESSED_FILE ${TEST_WORK_DIR}/${TEST_NAME}.decompressed)
    set(HFM_EXECUTABLE $<TARGET_FILE:hfm>)

    add_test(
        NAME hfm.roundtrip.${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -DORIG_FILE=${ORIG_FILE}
            -DTEST_WORK_DIR=${TEST_WORK_DIR}
            -DCOMPRESSED_FILE=${COMPRESSED_FILE}
            -DDECOMPRESSED_FILE=${DECOMPRESSED_FILE}
            -DHFM_EXECUTABLE=${HFM_EXECUTABLE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_test.cmake
    )
endforeach()
