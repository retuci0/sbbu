#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

enum class PauseAction { 
    NONE, 
    RESUME, 
    QUIT, 
    CHANGE_CHARACTERS, 
    CHANGE_VOLUME 
};

PauseAction drawPauseScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font,
                             const SDL_Event& event, int screenW, int screenH);