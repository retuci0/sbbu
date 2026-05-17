#pragma once

#include "../misc/common.h"

#include <SDL2/SDL.h>

#include <string>


class Projectile {
public:
    SDL_Rect rect = {};
    Facing direction;
    float velocity = 13.0f;

    SDL_Texture* img_left = nullptr;  // non-owning
    SDL_Texture* img_right = nullptr;

    Projectile(SDL_Texture* imgLeft, SDL_Texture* imgRight, int x, int y, Facing dir);

    void move();
    void draw(SDL_Renderer* r);
    void drawHitboxes(SDL_Renderer* r) const;
};