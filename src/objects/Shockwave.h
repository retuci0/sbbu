#pragma once

#include "CollisionRect.h"

#include "Player.h"
#include "../misc/Common.h"

#include <SDL2/SDL_rect.h>

#include <SDL2/SDL_render.h>
#include <optional>
#include <utility>


class ShockwaveRect : public CollisionRect {
public:
    float dx = 0.0f, dy = 0.0f;
    bool active = true;
    SDL_Rect prevRect = {};

    ShockwaveRect(int x, int y, int w, int h, Player* owner)
    : CollisionRect(x, y, w, h, owner) {}

    void update(float ts);
};


class Shockwave {
public:
    Shockwave(int spawnX, int spawnY, Player* owner);

    void update(float ts);
    void draw(SDL_Renderer* r, float a);
    
    std::optional<Facing> checkCollision(const Player& player);

    bool isAlive() const { return rects.first.active || rects.second.active; }
    Player* getOwner() const { return owner; }

    void drawHitboxes(SDL_Renderer* renderer, float a);

private:
    Player* owner;
    SDL_Texture* img;
    std::pair<ShockwaveRect, ShockwaveRect> rects;
};