# Blacksite Engine Library
file(GLOB_RECURSE BLACKSITE_SOURCES "src/*.cpp")
file(GLOB_RECURSE BLACKSITE_HEADERS "include/blacksite/*.h")

add_library(BlacksiteEngine STATIC ${BLACKSITE_SOURCES})

target_include_directories(BlacksiteEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
