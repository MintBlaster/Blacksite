# Editor executable
file(GLOB_RECURSE EDITOR_SOURCES "*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "*.h")

add_executable(BlacksiteEditor ${EDITOR_SOURCES})

target_include_directories(BlacksiteEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BlacksiteEditor PRIVATE
    BlacksiteEngine
)

target_compile_options(BlacksiteEditor PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)
