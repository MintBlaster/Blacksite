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

# Third-party libraries setup
include(cmake/ThirdParty.cmake)

# Add subdirectories
add_subdirectory(blacksite)
add_subdirectory(editor)
# add_subdirectory(examples)

message(STATUS "Blacksite Engine configured")
