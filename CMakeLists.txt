cmake_minimum_required(VERSION 3.16)
project(super_bert_bros VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- native build ---
if(NOT EMSCRIPTEN)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2       REQUIRED sdl2)
    pkg_check_modules(SDL2_image REQUIRED SDL2_image)
    pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
    pkg_check_modules(SDL2_ttf   REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_net   REQUIRED SDL2_net)

    # discord rpc
    set(CLANG_FORMAT_CMD "" CACHE STRING "" FORCE)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(lib/discord-rpc)
    target_compile_options(discord-rpc PRIVATE -Wno-error -Wno-template-body)
endif()

# --- wasm build ---
if(EMSCRIPTEN)
    set(USE_SDL2_NET ON)
endif()



# automatically find all source files
file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.c"
)

# also collect headers
file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
)

# windows resource file (icon)
if(NOT EMSCRIPTEN AND (MINGW OR WIN32))
    list(APPEND SRC icon.rc)
endif()

add_executable(super_bert_bros ${SRC} ${HEADERS})

# include dirs
target_include_directories(super_bert_bros PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

if(NOT EMSCRIPTEN)
    target_include_directories(super_bert_bros PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/include
    )
endif()

# --- link libs ---
if(NOT EMSCRIPTEN)
    target_link_libraries(super_bert_bros
        ${SDL2_LIBRARIES}
        ${SDL2_image_LIBRARIES}
        ${SDL2_mixer_LIBRARIES}
        ${SDL2_ttf_LIBRARIES}
        ${SDL2_net_LIBRARIES}
        discord-rpc
    )
    if(MINGW OR WIN32)
        target_link_libraries(super_bert_bros ws2_32)
    endif()
else()
    target_link_options(super_bert_bros PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_IMAGE=2
        -sSDL2_IMAGE_FORMATS=["png","jpg","bmp"]
        -sUSE_SDL_MIXER=2
        -sSDL2_MIXER_FORMATS=["wav","mp3","ogg"]
        -sUSE_SDL_TTF=2
        $<$<BOOL:${USE_SDL2_NET}>:-sUSE_SDL_NET=2>
        -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0
        -O3
        -sWASM=1
        -o ${PROJECT_NAME}.html
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets
    )
endif()

# -- compile options ---
if(NOT EMSCRIPTEN)
    target_compile_options(super_bert_bros PRIVATE
        ${SDL2_CFLAGS_OTHER}
        ${SDL2_image_CFLAGS_OTHER}
        ${SDL2_mixer_CFLAGS_OTHER}
        ${SDL2_ttf_CFLAGS_OTHER}
        ${SDL2_net_CFLAGS_OTHER}
    )
else()
    # emscripten specific compile options
    target_compile_options(super_bert_bros PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_IMAGE=2
        -sUSE_SDL_MIXER=2
        -sUSE_SDL_TTF=2
        $<$<BOOL:${USE_SDL2_NET}>:-sUSE_SDL_NET=2>
        -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0
        -O3
    )
endif()