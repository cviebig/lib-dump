cmake_minimum_required(VERSION 3.1)
project(lib-dump)


# Boost
find_package( Boost 1.61.0 )
include_directories( ${Boost_INCLUDE_DIR} )


# Library
add_library(dump INTERFACE)
target_include_directories(dump INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
export(TARGETS dump FILE dumpConfig.cmake)
export(PACKAGE dump)


# Catch
enable_testing(true)
set(TEST_ENABLED true)


# Tests
add_subdirectory(test)
