#pragma once

#include <SDL2/SDL.h>

#include <string>


enum class PlatformSize {
    BIG,
    SMALL
};

class Platform {
public:
    SDL_Texture* image = nullptr;  // non-owning
    SDL_Rect     rect  = {};

    Platform(SDL_Texture* bigTex, SDL_Texture* smallTex, int x, int y, int w, int h, PlatformSize size);

    void draw(SDL_Renderer* r) const;
    void drawHitboxes(SDL_Renderer* r) const;
};