# Set minimal CMake version
cmake_minimum_required(VERSION 3.0)

# Set project name
project(wtime)

# Set default C++ version
set(CMAKE_CXX_STANDARD 14)

# Generate executable
add_executable(wtime time.cpp)
