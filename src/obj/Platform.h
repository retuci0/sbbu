#pragma once

#include <SDL2/SDL.h>


enum class PlatformSize {
    BIG,
    SMALL
};

class Platform {
public:
    SDL_Texture* image = nullptr;  // non-owning
    SDL_Rect rect = {};
    PlatformSize size;

    Platform(int x, int y, int w, int h, PlatformSize size);

    void draw(SDL_Renderer* r, float a) const;
    void drawHitbox(SDL_Renderer* r, float a) const;
};