
cmake_minimum_required(VERSION 3.10)

project(gtl VERSION 0.1.0 LANGUAGES C CXX) # GUI tool kit

# For C projects
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

glad_add_library(glad_gl_core_33 STATIC
    API gl:core=3.3
)

# Embed shaders into a generated header at build time.
set(GENERATED_SHADERS_DIR ${CMAKE_BINARY_DIR}/generated)
set(GENERATED_SHADERS_HEADER ${GENERATED_SHADERS_DIR}/shaders_embedded.h)

add_custom_command(
    OUTPUT ${GENERATED_SHADERS_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -DBASE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders
        -DSHADERS=triangle.vert:VERTEX_SHADER,triangle.frag:FRAGMENT_SHADER
        -DOUTPUT=${GENERATED_SHADERS_HEADER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
    DEPENDS shaders/triangle.vert shaders/triangle.frag
    COMMENT "Embedding shaders into shaders_embedded.h"
)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/gtl.c
        ${GENERATED_SHADERS_HEADER}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        src/
        ${GENERATED_SHADERS_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        crox
    PRIVATE
        glfw
        glad_gl_core_33
)
