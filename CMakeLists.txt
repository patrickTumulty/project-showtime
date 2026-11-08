
cmake_minimum_required(VERSION 3.10)

project(project-showtime VERSION 0.1.0 LANGUAGES C CXX)

# For C projects
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(extern/crox)
add_subdirectory(extern/glfw)
add_subdirectory(extern/glad/cmake)

add_subdirectory(libs/gtl)

file(GLOB SOURCES src/main.c)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        gtl
)

