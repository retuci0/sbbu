#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


struct VolumeResult {
    float sfx;    // 0.0 – 2.0
    float music;  // 0.0 – 2.0
};

VolumeResult runVolumeScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font,
                              float currentSfx, float currentMusic);