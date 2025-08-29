cmake_minimum_required(VERSION 3.24)
project(test_agentcpp)

# Find the AgentCpp package
find_package(agentcpp REQUIRED)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create test executable
add_executable(test_agentcpp test_agentcpp.cpp)

# Link with AgentCpp
target_link_libraries(test_agentcpp agentixx::agentcpp)

# Enable testing
enable_testing()
add_test(NAME test_agentcpp COMMAND test_agentcpp)