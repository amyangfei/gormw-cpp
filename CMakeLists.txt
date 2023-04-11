cmake_minimum_required(VERSION 3.14)
project(gormw-cpp)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

######################################################################################################################
# ADD DEPENDENCIES
######################################################################################################################

include(cmake/CPM.cmake)

######################################################################################################################
# COMPILER SETUP
######################################################################################################################

if (NOT GOR_SANITIZER)
    set(GOR_SANITIZER address)
endif ()

message("Build mode: ${CMAKE_BUILD_TYPE}")
message("${GOR_SANITIZER} sanitizer will be enabled in debug mode.")

# Compiler flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-parameter -Wno-attributes") #TODO: remove
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=${GOR_SANITIZER} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

# Output directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes.
set(GOR_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include ${PROJECT_SOURCE_DIR}/src/include/gzip)

include_directories(${GOR_SRC_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

######################################################################################################################
# Other CMake modules
# MUST BE ADDED AFTER CONFIGURING COMPILER PARAMETERS
######################################################################################################################

add_subdirectory(src)
add_subdirectory(tests)

add_dependencies(gtest gor)