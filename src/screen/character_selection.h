#pragma once

#include "../misc/characters.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


struct SelectionResult {
    const Character* char1;
    const Character* char2;
    std::string      name1;
    std::string      name2;
};

SelectionResult runCharacterSelection(
    SDL_Renderer*      renderer,
    TTF_Font*          titleFont,
    TTF_Font*          font,
    const Character*   chars[4],
    const std::string& defaultName1,
    const Character*   defaultChar1,
    const std::string& defaultName2,
    const Character*   defaultChar2
);