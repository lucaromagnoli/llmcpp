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

# Unit test files
set(UNIT_TEST_SOURCES
    unit/test_types.cpp
    unit/test_client.cpp
    unit/test_llm_types.cpp
    unit/test_openai_types.cpp  # Re-enabling to check errors
    unit/test_openai_client.cpp  # Re-enabling to check errors
    unit/test_openai_response_parsing.cpp
    unit/test_client_factory.cpp
    unit/test_model_enum.cpp
)

# Integration test files
set(INTEGRATION_TEST_SOURCES
    integration/test_simple_integration.cpp
    integration/test_openai_integration.cpp  # Re-enabling to check errors
)

# Create test executable with all tests
add_executable(llmcpp_tests
    ${UNIT_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
    bench/benchmark_core.cpp
    bench/test_benchmarks.cpp
)

# Link against the library and test framework
target_link_libraries(llmcpp_tests PRIVATE
    llmcpp
    Catch2::Catch2WithMain
)

# Include directories
target_include_directories(llmcpp_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/unit
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
)

# Add test to ctest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(llmcpp_tests)

# Optional: Create separate test executables for unit and integration tests
option(LLMCPP_SEPARATE_TEST_EXECUTABLES "Create separate executables for unit and integration tests" OFF)

if(LLMCPP_SEPARATE_TEST_EXECUTABLES)
    # Unit tests executable
    add_executable(llmcpp_unit_tests ${UNIT_TEST_SOURCES})
    target_link_libraries(llmcpp_unit_tests PRIVATE llmcpp Catch2::Catch2WithMain)
    target_include_directories(llmcpp_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/unit
    )
    catch_discover_tests(llmcpp_unit_tests TEST_PREFIX "unit.")

    # Integration tests executable
    add_executable(llmcpp_integration_tests ${INTEGRATION_TEST_SOURCES})
    target_link_libraries(llmcpp_integration_tests PRIVATE llmcpp Catch2::Catch2WithMain)
    target_include_directories(llmcpp_integration_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
    )
    catch_discover_tests(llmcpp_integration_tests TEST_PREFIX "integration.")
endif()