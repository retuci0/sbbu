#pragma once

#include "CollisionRect.h"

#include "Player.h"
#include "../misc/Common.h"
#include "../misc/Renderer.h"

#include <SDL2/SDL_rect.h>

#include <SDL2/SDL_render.h>
#include <optional>
#include <utility>


class ShockwaveRect : public CollisionRect {
public:
    float dx = 0.0f, dy = 0.0f;
    bool  active = true;

    ShockwaveRect(int x, int y, int w, int h, Player* owner)
        : CollisionRect(x, y, w, h, owner) {}

    void update() {
        if (!active) return;
        rect.x += static_cast<int>(dx);
        rect.y += static_cast<int>(dy);
        // deactivate once the rect leaves the screen
        if (rect.x + rect.w < 0 || rect.x > SW) active = false;
    }
};


class Shockwave {
public:
    Shockwave(int spawnX, int spawnY, Player* owner)
        : owner(owner),
          rects(ShockwaveRect(spawnX, spawnY, 32, 32, owner),
                ShockwaveRect(spawnX, spawnY, 32, 32, owner))
    {
        rects.first.dx  = -7.0f;   // travels left
        rects.second.dx =  7.0f;   // travels right
    }

    void update() {
        rects.first.update();
        rects.second.update();
    }

    std::optional<Facing> checkCollision(const Player& player) {
        if (rects.first.active  && SDL_HasIntersection(&player.rect, &rects.first.rect)) {
            rects.first.active = false;
            return Facing::LEFT;
        }
        if (rects.second.active && SDL_HasIntersection(&player.rect, &rects.second.rect)) {
            rects.second.active = false;
            return Facing::RIGHT;
        }
        return std::nullopt;
    }

    bool isAlive() const { return rects.first.active || rects.second.active; }
    Player* getOwner() const { return owner; }

    void drawHitboxes(SDL_Renderer* renderer) {
        Renderer::outlineRect(renderer, rects.first.rect.x, rects.first.rect.y, rects.first.rect.w, rects.first.rect.h, GREEN, 2);
        Renderer::outlineRect(renderer, rects.second.rect.x, rects.second.rect.y, rects.second.rect.w, rects.second.rect.h, GREEN, 2);
    }

private:
    Player* owner;
    std::pair<ShockwaveRect, ShockwaveRect> rects;
};