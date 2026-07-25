cmake_minimum_required(VERSION 3.14)
project(cache_simulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra -O2)

include_directories(include)

add_executable(cache_sim
    src/main.cpp
    src/cache_level.cpp
    src/cache_hierarchy.cpp
    src/trace_reader.cpp
)
