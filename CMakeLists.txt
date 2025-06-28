cmake_minimum_required(VERSION 3.16)
project(Blacksite VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(assimp REQUIRED)

# Find Bullet Physics using pkg-config (more reliable on Arch)
pkg_check_modules(BULLET REQUIRED bullet)

# Include directories
include_directories(include)
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(/usr/include/bullet)

# Source files (will be empty initially)
file(GLOB_RECURSE BLACKSITE_SOURCES "src/*/*.cpp")
file(GLOB_RECURSE BLACKSITE_HEADERS "include/blacksite/*/*.h")

# Test executable to verify setup
add_executable(blacksite_test
    src/main.cpp
    ${BLACKSITE_SOURCES}
)

# Link libraries
target_link_libraries(blacksite_test
    OpenGL::GL
    glfw
    GLEW::GLEW
    ${BULLET_LIBRARIES}
    assimp::assimp
)

# Compiler flags
target_compile_options(blacksite_test PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Print setup info
message(STATUS "Blacksite Engine Setup")
message(STATUS "OpenGL found: ${OPENGL_FOUND}")
message(STATUS "GLFW found: ${glfw3_FOUND}")
message(STATUS "GLEW found: ${GLEW_FOUND}")
message(STATUS "Bullet include: ${BULLET_INCLUDE_DIRS}")
message(STATUS "Bullet libraries: ${BULLET_LIBRARIES}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")