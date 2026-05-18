#pragma once

#include "../misc/Common.h"

#include <SDL2/SDL.h>



class Projectile {
public:
    SDL_Rect rect = {};
    Facing direction;
    float velocity = 13.0f;

    SDL_Texture* img = nullptr;  // non-owning

    Projectile(SDL_Texture* img, int x, int y, Facing dir);

    void move();
    void draw(SDL_Renderer* r);
    void drawHitboxes(SDL_Renderer* r) const;
};