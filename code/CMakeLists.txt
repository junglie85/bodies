cmake_minimum_required(VERSION 3.30)
project(bodies LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(SDL3 REQUIRED PATHS ../installed/cmake)

add_executable(bodies
        main.c
        application.c
        error.h
        log.c
)

target_sources(bodies PUBLIC
        application.h
        log.h
)

# Windows
set(LIBS Winmm SetupAPI Imm32 Version)
target_link_libraries(bodies PUBLIC SDL3::SDL3-static ${LIBS})

target_compile_options(bodies PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FC /Zi /W4 /WX /external:anglebrackets /external:W0>
        # $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra -Werror>
)
