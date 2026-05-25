#pragma once

#include "../misc/Common.h"
#include "../objects/Player.h"

#include <SDL2/SDL.h>


class Projectile {
public:
    SDL_Rect rect = {};
    Facing direction;
    float velocity = 19.0f;
    Player* owner = nullptr;  // non-owning

    SDL_Texture* img = nullptr;  // non-owning

    Projectile(SDL_Texture* img, int x, int y, Facing dir, Player* owner);

    void move();
    void draw(SDL_Renderer* r);
    void drawHitbox(SDL_Renderer* r) const;
};