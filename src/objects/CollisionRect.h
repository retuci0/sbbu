#pragma once

#include "../objects/Player.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


class CollisionRect {
public:
    CollisionRect(int x, int y, int w, int h, Player* owner, int durationFrames = 5)
        : rect({x, y, w, h}), owner(owner), lifetime(durationFrames) {}

    SDL_Rect rect = {};
    Player* owner;
    int lifetime = 0;

    void update() { if (lifetime > 0) --lifetime; }
    bool isAlive() const { return lifetime > 0; }
    void drawHitbox(SDL_Renderer* r);
};