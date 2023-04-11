cmake_minimum_required(VERSION 3.14)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.13.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

enable_testing()

file(GLOB_RECURSE GOR_TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*test.cpp")

# #####################################################################################################################
# MAKE TARGETS
# #####################################################################################################################

# #########################################
# "make check-tests"
# #########################################
add_custom_target(build-tests COMMAND ${CMAKE_CTEST_COMMAND} --show-only)

# #########################################
# "make XYZ_test"
# #########################################
foreach (gor_test_source ${GOR_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(gor_test_filename ${gor_test_source} NAME)
    string(REPLACE ".cpp" "" gor_test_name ${gor_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${gor_test_name} EXCLUDE_FROM_ALL ${gor_test_source})
    add_dependencies(build-tests ${gor_test_name})

    gtest_discover_tests(${gor_test_name}
            EXTRA_ARGS
            --gtest_color=auto
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${gor_test_name}.xml
            --gtest_catch_exceptions=0
            DISCOVERY_TIMEOUT 120
            PROPERTIES
            TIMEOUT 120
            )

    target_link_libraries(${gor_test_name} gor gtest gmock_main z)

    # Set test target properties and dependencies.
    set_target_properties(${gor_test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${gor_test_name}
            )
endforeach ()
