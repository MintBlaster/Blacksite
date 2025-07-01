cmake_minimum_required(VERSION 3.16)
project(Blacksite VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build typed
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

# Jolt Physics setup
set(JOLT_DIR ${CMAKE_SOURCE_DIR}/third_party/JoltPhysics)
set(JOLT_SRC_DIR ${JOLT_DIR}/Jolt)
file(GLOB_RECURSE JOLT_SOURCES ${JOLT_SRC_DIR}/*.cpp)
add_library(JoltPhysics STATIC ${JOLT_SOURCES})
target_include_directories(JoltPhysics PUBLIC ${JOLT_DIR} ${JOLT_SRC_DIR})
target_compile_options(JoltPhysics PRIVATE -w)

# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Create Blacksite Engine Library
add_library(BlacksiteEngine STATIC ${BLACKSITE_SOURCES})

target_include_directories(BlacksiteEngine PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(BlacksiteEngine PUBLIC
    OpenGL::GL
    glfw
    GLEW::GLEW
    JoltPhysics
    assimp::assimp
    imgui
)

target_compile_definitions(BlacksiteEngine PUBLIC
    GLEW_STATIC
    GLM_ENABLE_EXPERIMENTAL
)

target_compile_options(BlacksiteEngine PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Main test executable
add_executable(blacksite_test src/main.cpp)
target_link_libraries(blacksite_test BlacksiteEngine)

# Main executable only

message(STATUS "Blacksite Engine configured")
