cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
project(bodies LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

find_package(SDL3 REQUIRED PATHS ../installed/cmake)

add_executable(bodies
        main.cpp
        platform.cpp
        platform.window.cpp
)

target_sources(bodies PUBLIC
        FILE_SET CXX_MODULES FILES
        platform.ixx
        platform.window.ixx
)

# Windows
set(LIBS Winmm SetupAPI Imm32 Version)
target_link_libraries(bodies PUBLIC SDL3::SDL3-static ${LIBS})

target_compile_options(bodies PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FC /Zi /W4 /WX /external:anglebrackets /external:W0>
        # $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra -Werror>
)
