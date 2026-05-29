cmake_minimum_required(VERSION 3.16)
project(super_bert_bros VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig       REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_image REQUIRED SDL2_image)
pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_ttf   REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_net   REQUIRED SDL2_net)

# automatically find all .CPP files
file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.c"
)

# also collect headers
file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
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