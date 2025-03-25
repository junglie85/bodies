cmake_minimum_required(VERSION 3.30)
project(bodies VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(BodiesVendor REQUIRED PATHS ../installed/cmake)
find_package(SDL3 REQUIRED PATHS ../installed/cmake)

add_executable(bodies
        main.c
        application.c
        application.h
        error.h
        log.c
        log.h
        window.c
        window.h
)

set(LIBS bodies::vendor SDL3::SDL3-static)

# Windows
list(APPEND LIBS Winmm SetupAPI Imm32 Version)
target_link_libraries(bodies PUBLIC ${LIBS})

target_compile_options(bodies PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FC /Zi /W4 /WX /external:anglebrackets /external:W0>
        # $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra -Werror>
)
