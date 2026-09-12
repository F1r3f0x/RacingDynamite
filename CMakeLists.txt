cmake_minimum_required(VERSION 3.20)
project(RacingDynamite C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
)

# Audio subsystem sources
set(AUDIO_SOURCES
    src/audio/audio.c
    src/audio/sound_pool.c
    src/audio/engine_audio.c
    src/audio/music.c
)

# Source files
set(SOURCES
    src/core/main.c
    src/core/log.c
    src/core/game_state.c
    src/formats/pic.c
    src/formats/col.c
    src/formats/srf.c
    src/formats/plc.c
    src/formats/msh.c
    src/formats/tex_tab.c
    src/formats/lft.c
    src/formats/tri.c
    src/formats/pos.c
    src/formats/cdp.c
    src/renderer/camera.c
    src/renderer/rasterizer.c
    src/physics/getsurf.c
    src/physics/vehicle.c
    src/platform/platform_sdl.c
    ${AUDIO_SOURCES}
)

add_executable(racing_dynamite ${SOURCES})

# SDL2 Link configuration
if (MSVC)
    set(SDL2_LIBRARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2main.lib
    )
else()
    # MinGW / GCC / Clang
    set(SDL2_LIBRARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.lib
    )
endif()

target_link_libraries(racing_dynamite PRIVATE ${SDL2_LIBRARIES})

# Post-build: copy SDL2.dll to build directory
add_custom_command(TARGET racing_dynamite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdl2/lib/x64/SDL2.dll
    $<TARGET_FILE_DIR:racing_dynamite>
)

# Test executable for formats
add_executable(test_formats
    tests/test_formats.c
    src/core/log.c
    src/formats/pic.c
    src/formats/col.c
    src/formats/srf.c
    src/formats/plc.c
    src/formats/msh.c
    src/formats/tex_tab.c
    src/formats/lft.c
    src/formats/tri.c
    src/formats/pos.c
)

# Test executable for game state machine and renderer
add_executable(test_game_states
    tests/test_game_states.c
    src/core/log.c
    src/core/game_state.c
    src/formats/pic.c
    src/formats/col.c
    src/formats/srf.c
    src/formats/plc.c
    src/formats/msh.c
    src/formats/tex_tab.c
    src/formats/lft.c
    src/formats/tri.c
    src/formats/pos.c
    src/renderer/camera.c
    src/renderer/rasterizer.c
    src/physics/getsurf.c
    src/physics/vehicle.c
    ${AUDIO_SOURCES}
)
target_link_libraries(test_game_states PRIVATE ${SDL2_LIBRARIES})

# Test executable for vehicle physics and surface raycast simulation
add_executable(test_physics
    tests/test_physics.c
    src/core/log.c
    src/core/game_state.c
    src/physics/getsurf.c
    src/physics/vehicle.c
    src/formats/srf.c
    src/formats/plc.c
    src/formats/msh.c
    src/formats/tex_tab.c
    src/formats/lft.c
    src/formats/tri.c
    src/formats/pos.c
    src/formats/col.c
    src/formats/pic.c
    src/renderer/camera.c
    src/renderer/rasterizer.c
    ${AUDIO_SOURCES}
)
target_link_libraries(test_physics PRIVATE ${SDL2_LIBRARIES})

# Test executable for camera subsystem
add_executable(test_camera
    tests/test_camera.c
    src/core/log.c
    src/renderer/camera.c
)

# Test executable for audio subsystem
add_executable(test_audio
    tests/test_audio.c
    src/core/log.c
    ${AUDIO_SOURCES}
)
target_link_libraries(test_audio PRIVATE ${SDL2_LIBRARIES})

enable_testing()
add_test(NAME TestFormats COMMAND test_formats)
add_test(NAME TestGameStates COMMAND test_game_states)
add_test(NAME TestPhysics COMMAND test_physics)
add_test(NAME TestCamera COMMAND test_camera)
add_test(NAME TestAudio COMMAND test_audio)

# Automated Fidelity & Parity Audit Test (Phase 4 / FCTS)
find_package(Python3 COMPONENTS Interpreter QUIET)
if (Python3_FOUND)
    add_test(NAME VerifyFidelity COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_fidelity.py)
else()
    add_test(NAME VerifyFidelity COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_fidelity.py)
endif()
