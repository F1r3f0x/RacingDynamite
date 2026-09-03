cmake_minimum_required(VERSION 3.20)
project(RacingDynamite C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/include
)

# Source files
set(SOURCES
    src/core/main.c
    src/formats/pic.c
    src/formats/col.c
    src/platform/platform_sdl.c
)

add_executable(racing_dynamite ${SOURCES})

# SDL2 Link configuration
if (MSVC)
    target_link_libraries(racing_dynamite PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2main.lib
    )
else()
    # MinGW / GCC / Clang
    target_link_libraries(racing_dynamite PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.lib
    )
endif()

# Post-build: copy SDL2.dll to build directory
add_custom_command(TARGET racing_dynamite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.dll
    $<TARGET_FILE_DIR:racing_dynamite>
)

# Test executable for formats
add_executable(test_formats
    tests/test_formats.c
    src/formats/pic.c
    src/formats/col.c
)

