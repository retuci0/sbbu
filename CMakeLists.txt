cmake_minimum_required(VERSION 3.16)
project(super_bert_bros VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_image REQUIRED SDL2_image)
pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_ttf  REQUIRED SDL2_ttf)

set(SRC
    src/main.cpp
    src/game.cpp
    src/input_handler.cpp
    src/misc/characters.cpp
    src/misc/common.cpp
    src/objects/player.cpp
    src/objects/projectile.cpp
    src/objects/platform.cpp
    src/screen/character_selection.cpp
    src/screen/pause.cpp
    src/screen/volume.cpp
)

set(HEADERS
    src/game.h
    src/input_handler.h
    src/misc/characters.h
    src/misc/common.h
    src/objects/player.h
    src/objects/projectile.h
    src/objects/platform.h
    src/screen/character_selection.h
    src/screen/pause.h
    src/screen/volume.h
)

add_executable(super_bert_bros ${SRC} ${HEADERS})

target_include_directories(super_bert_bros PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(super_bert_bros
    ${SDL2_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
)

target_compile_options(super_bert_bros PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_image_CFLAGS_OTHER}
    ${SDL2_mixer_CFLAGS_OTHER}
    ${SDL2_ttf_CFLAGS_OTHER}
)

message(STATUS "build with: cmake --build build")
message(STATUS "run from the repo root: ./build/super_bert_bros")