# Tests for llmcpp
cmake_minimum_required(VERSION 3.22)

# Find or fetch testing framework
include(FetchContent)

# Use doctest for testing (lightweight and header-only)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

# Create test executable
add_executable(llmcpp_tests
    test_types.cpp
    test_client.cpp
)

# Link against the library and test framework
target_link_libraries(llmcpp_tests PRIVATE
    llmcpp
    doctest::doctest
)

# Include directories
target_include_directories(llmcpp_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add test to ctest
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(llmcpp_tests) 