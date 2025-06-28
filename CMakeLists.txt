cmake_minimum_required(VERSION 3.16)
project(Blacksite VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

# Source files
file(GLOB_RECURSE BLACKSITE_SOURCES "src/*/*.cpp")
file(GLOB_RECURSE BLACKSITE_HEADERS "include/blacksite/*/*.h")

# -------------------------------
# Jolt Physics setup
set(JOLT_DIR ${CMAKE_SOURCE_DIR}/third_party/JoltPhysics)
set(JOLT_SRC_DIR ${JOLT_DIR}/Jolt)

file(GLOB_RECURSE JOLT_SOURCES
    ${JOLT_SRC_DIR}/*.cpp
)

add_library(JoltPhysics STATIC ${JOLT_SOURCES})

target_include_directories(JoltPhysics PUBLIC
    ${JOLT_DIR}
    ${JOLT_SRC_DIR}
)

# Optional: silence warnings from Jolt
target_compile_options(JoltPhysics PRIVATE -w)
# -------------------------------

# Test executable
add_executable(blacksite_test
    src/main.cpp
    ${BLACKSITE_SOURCES}
)

target_include_directories(blacksite_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(blacksite_test
    OpenGL::GL
    glfw
    GLEW::GLEW
    JoltPhysics
    assimp::assimp
)

# Compiler flags
target_compile_options(blacksite_test PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Setup summary
message(STATUS "Blacksite Engine Setup")
message(STATUS "OpenGL found: ${OPENGL_FOUND}")
message(STATUS "GLFW found: ${glfw3_FOUND}")
message(STATUS "GLEW found: ${GLEW_FOUND}")
message(STATUS "Assimp found: ${assimp_FOUND}")
message(STATUS "Jolt integrated from: ${JOLT_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
