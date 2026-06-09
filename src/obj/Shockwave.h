#pragma once

#include "CollisionRect.h"

#include "obj/Entity.h"
#include "obj/Player.h"
#include "misc/Common.h"

#include <SDL2/SDL_rect.h>

#include <SDL2/SDL_render.h>
#include <optional>
#include <utility>


constexpr int SHOCKWAVE_SIZE = 32;
constexpr float SHOCKWAVE_SPEED = 7.0f;

class ShockwaveRect : public CollisionRect {
public:
    bool active = true;

    ShockwaveRect(int x, int y, int w, int h, Player* owner)
    : CollisionRect(x, y, w, h, owner) {}

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
};


class Shockwave : public Entity {
public:
    Shockwave(int spawnX, int spawnY, Player* owner);

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
    
    std::optional<Facing> checkCollision(const Player& player);
    
    bool isAlive() const { return rects.first.active || rects.second.active; }
    Player* getOwner() const { return owner; }
    
    void draw(SDL_Renderer* r, float a) override;
    void drawHitbox(SDL_Renderer* renderer, float a) const override;

    EntityType getType() const override {
        return EntityType::MISC;
    }

private:
    Player* owner;
    std::pair<ShockwaveRect, ShockwaveRect> rects;
};