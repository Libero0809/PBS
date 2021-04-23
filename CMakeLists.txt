cmake_minimum_required(VERSION 3.10)

project(PBS_DEMO VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

add_executable(demo src/demo.cpp)

target_include_directories(demo PUBLIC ./include)
target_link_directories(demo PUBLIC ./lib)
target_link_libraries(demo PUBLIC xxhash minisketch)
target_compile_options(demo PUBLIC -O2)

set_target_properties(demo PROPERTIES OUTPUT_NAME "demo")