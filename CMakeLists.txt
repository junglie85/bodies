cmake_minimum_required(VERSION 3.30)
project(bodies)

set(CMAKE_CXX_STANDARD 26)

add_executable(bodies main.cpp platform.window.cpp)
target_sources(bodies PUBLIC
        FILE_SET CXX_MODULES FILES
        platform.window.ixx
)
