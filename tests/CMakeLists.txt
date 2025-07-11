# Tests for llmcpp
cmake_minimum_required(VERSION 3.22)

# Find or fetch testing framework
include(FetchContent)

# Use Catch2 for testing (industry standard)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Create test executable
add_executable(llmcpp_tests
    test_types.cpp
    test_client.cpp
    # test_openai_types.cpp
    test_llm_types.cpp
    # test_openai_client.cpp
    test_client_factory.cpp
)

# Link against the library and test framework
target_link_libraries(llmcpp_tests PRIVATE
    llmcpp
    Catch2::Catch2WithMain
)

# Include directories
target_include_directories(llmcpp_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add test to ctest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(llmcpp_tests) 