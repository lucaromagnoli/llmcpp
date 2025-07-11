# Examples for llmcpp
cmake_minimum_required(VERSION 3.22)

# Create example executables
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE llmcpp)

add_executable(async_example async_example.cpp)
target_link_libraries(async_example PRIVATE llmcpp)

# Install examples (optional)
install(TARGETS basic_usage async_example
    DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)