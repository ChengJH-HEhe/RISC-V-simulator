cmake_minimum_required(VERSION 3.5.0)
project(tomasulo VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O2 -O3 -Ofast")
# undefined reliance how to configure

include_directories(./src/)
file(GLOB_RECURSE main_src src/*.cpp)
add_executable(code ${main_src}) 

