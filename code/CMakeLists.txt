cmake_minimum_required(VERSION 3.30)
project(bodies VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(FEATURE_MEMORY_STATS "Record memory usage statistics" OFF)

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
        memory.c
        memory.h
)

set(LIBS bodies::vendor SDL3::SDL3-static)

# Windows
list(APPEND LIBS Winmm SetupAPI Imm32 Version)
target_link_libraries(bodies PUBLIC ${LIBS})

target_compile_options(bodies PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/FC /Zi /W4 /WX /external:anglebrackets /external:W0>
        # $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra -Werror>
)

if (FEATURE_MEMORY_STATS)
    target_compile_definitions(bodies PRIVATE FEATURE_MEMORY_STATS)
endif ()

add_dependencies(bodies shaders)

###################### Shaders ######################
find_program(SDL_SHADERCROSS shadercross PATH ../installed/bin)
file (GLOB_RECURSE SHADER_SOURCE_FILES ${PROJECT_SOURCE_DIR}/../data/*.hlsl)
foreach(SHADER_SOURCE ${SHADER_SOURCE_FILES})
    get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME_WLE)
    set(COMPILED_SHADER ${PROJECT_SOURCE_DIR}/../data/${FILE_NAME}.spv)
    add_custom_command(OUTPUT ${COMPILED_SHADER}
            COMMAND ${SDL_SHADERCROSS} ${SHADER_SOURCE} -o ${COMPILED_SHADER}
            DEPENDS ${SHADER_SOURCE})
    list(APPEND COMPILED_SHADERS ${COMPILED_SHADER})
endforeach ()

add_custom_target(shaders DEPENDS ${COMPILED_SHADERS})
