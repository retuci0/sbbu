cmake_minimum_required(VERSION 3.16)
project(super_bert_bros VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig       REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_image REQUIRED SDL2_image)
pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_ttf   REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_net   REQUIRED SDL2_net)


set(SRC
    src/Main.cpp
    src/Game.cpp
    src/InputHandler.cpp
    src/misc/Characters.cpp
    src/misc/Renderer.cpp
    src/net/ByteBuffer.cpp
    src/net/Network.cpp
    src/objects/CollisionRect.cpp
    src/objects/Player.cpp
    src/objects/Projectile.cpp
    src/objects/Platform.cpp
    src/ui/screen/CharacterSelectionScreen.cpp
    src/ui/screen/MultiplayerModeScreen.cpp
    src/ui/screen/PauseScreen.cpp
    src/ui/screen/RemoteSetupScreen.cpp
    src/ui/screen/VolumeScreen.cpp
    src/ui/screen/WaitingScreen.cpp

    inc/tinyfiledialogs.c
)

set(HEADERS
    src/Game.h
    src/InputHandler.h
    src/misc/Characters.h
    src/misc/Color.h
    src/misc/Common.h
    src/misc/Renderer.h
    src/net/ByteBuffer.h
    src/net/Network.h
    src/net/PacketRegistry.h
    src/net/Packets.h
    src/objects/CollisionRect.h
    src/objects/Player.h
    src/objects/Projectile.h
    src/objects/Platform.h
    src/objects/Shockwave.h
    src/ui/Screen.h
    src/ui/Widget.h
    src/ui/widget/Button.h
    src/ui/widget/Slider.h
    src/ui/screen/CharacterSelectionScreen.h
    src/ui/screen/MultiplayerModeScreen.h
    src/ui/screen/PauseScreen.h
    src/ui/screen/RemoteSetupScreen.h
    src/ui/screen/VolumeScreen.h
    src/ui/screen/WaitingScreen.h

    inc/tinyfiledialogs.h
)

enable_language(C)

add_executable(super_bert_bros ${SRC} ${HEADERS})

target_include_directories(super_bert_bros PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(super_bert_bros
    ${SDL2_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
    ${SDL2_net_LIBRARIES}
)
if (MINGW OR WIN32)
    target_link_libraries(super_bert_bros ws2_32)
endif()

target_compile_options(super_bert_bros PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2_image_CFLAGS_OTHER}
    ${SDL2_mixer_CFLAGS_OTHER}
    ${SDL2_ttf_CFLAGS_OTHER}
    ${SDL2_net_CFLAGS_OTHER}
)

message(STATUS "build with: cmake --build build")
message(STATUS "run from the repo root: ./build/super_bert_bros")