cmake_minimum_required(VERSION 3.30)
project(bodies LANGUAGES C)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL3 REQUIRED PATHS ../installed/cmake)

add_executable(bodies
        main.c
        platform.c
        platform.window.c
)

target_sources(bodies PUBLIC
        core.log.h
        platform.h
        platform.window.h
)

# Windows
set(LIBS Winmm SetupAPI Imm32 Version)
target_link_libraries(bodies PUBLIC SDL3::SDL3-static ${LIBS})

target_compile_options(bodies PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FC /Zi /W4 /WX /external:anglebrackets /external:W0>
        # $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra -Werror>
)
