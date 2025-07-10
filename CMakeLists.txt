cmake_minimum_required(VERSION 3.22)

project(juce-llm-client VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE dependency handling
find_package(PkgConfig REQUIRED)

# Option to build standalone (fetches JUCE) or as submodule
option(JUCE_LLM_CLIENT_FETCH_JUCE "Fetch JUCE automatically when building standalone" OFF)

# Try to find JUCE
find_package(juce CONFIG QUIET)

if(NOT juce_FOUND AND NOT TARGET juce::juce_core)
    if(JUCE_LLM_CLIENT_FETCH_JUCE OR CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        message(STATUS "JUCE not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.6
        )
        FetchContent_MakeAvailable(JUCE)
    else()
        message(STATUS "JUCE not found. Enable JUCE_LLM_CLIENT_FETCH_JUCE or use in a project with JUCE")
        return()
    endif()
endif()

# Create the library
add_library(juce_llm_client STATIC
    src/core/LLMTypes.cpp
    src/core/LLMClient.cpp
    src/openai/OpenAIClient.cpp
    src/openai/OpenAIModels.cpp
    src/openai/OpenAIUtils.cpp
    src/providers/ClientFactory.cpp
    src/providers/ClientManager.cpp
)

# Add include directories
target_include_directories(juce_llm_client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link JUCE modules
target_link_libraries(juce_llm_client PUBLIC
    juce::juce_core
)

# Compile definitions
target_compile_definitions(juce_llm_client PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISABLE_JUCE_VERSION_PRINTING=1
)

# Export the library for use by other projects
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(GNUInstallDirs)
    
    install(TARGETS juce_llm_client
        EXPORT juce-llm-client-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    
    install(EXPORT juce-llm-client-targets
        FILE juce-llm-client-targets.cmake
        NAMESPACE juce-llm-client::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/juce-llm-client
    )
    
    # Examples (optional)
    option(JUCE_LLM_CLIENT_BUILD_EXAMPLES "Build examples" OFF)
    if(JUCE_LLM_CLIENT_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    
    # Tests (optional)
    option(JUCE_LLM_CLIENT_BUILD_TESTS "Build tests" OFF)
    if(JUCE_LLM_CLIENT_BUILD_TESTS)
        add_subdirectory(tests)
    endif()
endif()
