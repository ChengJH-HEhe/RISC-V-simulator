cmake_minimum_required(VERSION 3.5.0)
project(tomasulo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -O3 -Ofast")

set(CMAKE_BUILD_TYPE Release)

# undefined reliance how to configure

include_directories(./src/)
file(GLOB_RECURSE main_src src/*.cpp)
add_executable(code ${main_src}) 

