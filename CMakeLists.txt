cmake_minimum_required(VERSION 3.22)

project(llmcpp VERSION 1.0.17 LANGUAGES CXX)

# Set version variables for easier access
set(LLMCPP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(LLMCPP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(LLMCPP_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(LLMCPP_VERSION ${PROJECT_VERSION})

# Generate version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/llmcpp_version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/llmcpp_version.h"
    @ONLY
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(LLMCPP_BUILD_TESTS "Build tests" OFF)
option(LLMCPP_BUILD_EXAMPLES "Build examples" OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(OpenSSL REQUIRED)

# Use FetchContent for dependencies
include(FetchContent)

# nlohmann-json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# cpp-httplib  
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)

# Configure httplib to use OpenSSL
set(HTTPLIB_USE_OPENSSL ON)

# Make dependencies available
FetchContent_MakeAvailable(nlohmann_json httplib)

# Source files
set(LLMCPP_SOURCES
    src/core/LLMTypes.cpp
    src/core/LLMClient.cpp
    src/core/JsonSchemaBuilder.cpp
    src/providers/ClientManager.cpp
    src/providers/ClientFactory.cpp
    src/openai/OpenAIClient.cpp
    src/openai/OpenAIHttpClient.cpp
    src/openai/OpenAIResponsesApi.cpp
    src/openai/OpenAISchemaBuilder.cpp
    src/openai/OpenAIModels.cpp
    src/openai/OpenAIUtils.cpp
)

# Create library
add_library(llmcpp ${LLMCPP_SOURCES})

# Add include directories
target_include_directories(llmcpp 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(llmcpp 
    PUBLIC 
        nlohmann_json
    PRIVATE
        httplib::httplib
        OpenSSL::SSL 
        OpenSSL::Crypto
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(llmcpp PRIVATE ws2_32 wsock32)
elseif(APPLE)
    target_link_libraries(llmcpp PRIVATE 
        "-framework CoreFoundation" 
        "-framework Security"
    )
endif()

# Add examples if requested
if(LLMCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Add tests if requested
if(LLMCPP_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install the library and its dependencies
install(TARGETS llmcpp nlohmann_json
    EXPORT llmcppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install generated version header
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/llmcpp_version.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export the targets to a script
install(EXPORT llmcppTargets
    FILE llmcppTargets.cmake
    NAMESPACE llmcpp::
    DESTINATION lib/cmake/llmcpp
)

# Generate and install the config and version files
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/llmcppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/llmcppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/llmcppConfig.cmake"
    INSTALL_DESTINATION lib/cmake/llmcpp
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/llmcppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/llmcppConfigVersion.cmake"
    DESTINATION lib/cmake/llmcpp
)

# CPack configuration for releases
include(CPack)

set(CPACK_PACKAGE_NAME "llmcpp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern C++20 library for Large Language Model APIs")
set(CPACK_PACKAGE_DESCRIPTION "A unified interface for Large Language Model APIs with support for multiple providers, async operations, and modern C++20 features.")
set(CPACK_PACKAGE_VENDOR "llmcpp")
set(CPACK_PACKAGE_CONTACT "https://github.com/lucaromagnoli/llmcpp")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Package file names
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Source package
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-source")
set(CPACK_SOURCE_IGNORE_FILES
    /.git/
    /build/
    /\.DS_Store
    /\.env
    /\.vscode/
    /\.idea/
    /cmake-build-*/
    /\.clion/
)

# Platform-specific settings
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_SOURCE_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SOURCE_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SOURCE_GENERATOR "TGZ")
endif()
