# Set minimal CMake version
cmake_minimum_required(VERSION 3.5)

# Set project name
project(wtime)

# Set default C++ version
set(CMAKE_CXX_STANDARD 11)

# Generate executable
add_executable(${PROJECT_NAME} wtime.cpp)
