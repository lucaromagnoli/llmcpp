cmake_minimum_required(VERSION 3.22)

project(llmcpp VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for dependencies
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_MakeAvailable(httplib)

# Create the library
add_library(llmcpp STATIC
    src/core/LLMTypes.cpp
    src/core/LLMClient.cpp
    src/openai/OpenAIClient.cpp
    src/openai/OpenAIModels.cpp
    src/openai/OpenAIUtils.cpp
    src/providers/ClientFactory.cpp
    src/providers/ClientManager.cpp
)

# Add include directories
target_include_directories(llmcpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(llmcpp PUBLIC
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Compile definitions for httplib
target_compile_definitions(llmcpp PUBLIC
    CPPHTTPLIB_OPENSSL_SUPPORT
)

# Find OpenSSL for HTTPS support
find_package(OpenSSL REQUIRED)
target_link_libraries(llmcpp PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Export the library for use by other projects
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)
    
    install(TARGETS llmcpp
        EXPORT llmcpp-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    
    install(EXPORT llmcpp-targets
        FILE llmcpp-targets.cmake
        NAMESPACE llmcpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/llmcpp
    )
    
    # Examples (optional)
    option(LLMCPP_BUILD_EXAMPLES "Build examples" OFF)
    if(LLMCPP_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    
    # Tests (optional)
    option(LLMCPP_BUILD_TESTS "Build tests" OFF)
    if(LLMCPP_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
