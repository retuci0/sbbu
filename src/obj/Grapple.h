#pragma once

#include "obj/GrapplePoint.h"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <vector>

class Player;
class Platform;
class Projectile;
class Item;

enum class GrappleState {
    FLYING,
    LATCHED_PLATFORM,
    LATCHED_PLAYER,
    LATCHED_PROJECTILE,
    LATCHED_POINT,
    LATCHED_ITEM,
    RETRACTING
};

struct Grapple {
    Grapple(Player& player, int startX, int startY, float velX, float velY);

    SDL_Rect  rect  = {};
    SDL_Rect  prevRect = {};
    float     dx    = 0.0f;
    float     dy    = 0.0f;
    Player&   owner;
    GrappleState state = GrappleState::FLYING;

    float playerDx0 = 0, playerDy0 = 0;

    Player*         targetPlayer     = nullptr;
    Projectile*     targetProjectile = nullptr;
    GrapplePoint*   targetPoint      = nullptr;
    Item*           targetItem       = nullptr;

    static constexpr float TRAVEL_SPEED  = 24.0f;
    static constexpr float PULL_FORCE    = 12.0f;
    static constexpr float MAX_RANGE     = 400.0f;
    static constexpr float RETRACT_SPEED = 10.0f;
    static constexpr float ARRIVE_DIST   = 64.0f;

    float originX = 0.0f;
    float originY = 0.0f;

    bool update(const std::vector<Platform>& platforms,
                std::vector<Player*>&        players,
                std::vector<Projectile>&     projectiles,
                std::vector<GrapplePoint>&   points,
                std::vector<std::unique_ptr<Item>>& items,
                float ts);

    void draw(SDL_Renderer* r, float a) const;
    void drawHitbox(SDL_Renderer* r, float a) const;
    

    bool isLatched() const;
    void retract();

private:
    float distanceFromOrigin() const;

    bool pullOwnerToward(float hx, float hy, float ts);
    bool pullItemTowardOwner(Item* item, float ts);
};