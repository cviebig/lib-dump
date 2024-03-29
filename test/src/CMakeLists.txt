cmake_minimum_required(VERSION 3.1)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

macro (add_test_target TEST_NAME TEST_SOURCE)
    if(TEST_ENABLED)
        add_executable(${TEST_NAME} ${TEST_SOURCE} "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")
        target_link_libraries(${TEST_NAME} dump Catch2::Catch2)
        target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                                                        "${CMAKE_CURRENT_SOURCE_DIR}/../include")

        set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 14)
        set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif(TEST_ENABLED)
endmacro()

macro(add_test_targets SUBDIRECTORY)
    file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIRECTORY}" "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIRECTORY}/*")
    add_test_target("${SUBDIRECTORY}${TEST}Test" "${TEST_FILES}")
endmacro()

add_test_targets("dump")
