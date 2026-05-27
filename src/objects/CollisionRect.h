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
    float lifetime = 0.0f;
    float damageScale = 1.0f;
    float kbScale     = 1.0f;

    void update(float ts) { if (lifetime > 0) lifetime -= ts; }
    bool isAlive() const { return lifetime > 0; }
    void drawHitbox(SDL_Renderer* r, float a);
};