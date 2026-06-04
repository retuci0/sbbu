#pragma once

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <vector>

class Player;
class Platform;
class Projectile;

enum class GrappleState {
    FLYING,
    LATCHED_PLATFORM,
    LATCHED_PLAYER,
    LATCHED_PROJECTILE,
    RETRACTING
};

struct Grapple {
    Grapple(Player& player, int startX, int startY, float velX, float velY);

    SDL_Rect  rect  = {};
    float     dx    = 0.0f;
    float     dy    = 0.0f;
    Player&   owner;
    GrappleState state = GrappleState::FLYING;

    Player*     targetPlayer     = nullptr;
    Projectile* targetProjectile = nullptr;

    static constexpr float TRAVEL_SPEED  = 24.0f;
    static constexpr float PULL_FORCE    = 12.0f;
    static constexpr float MAX_RANGE     = 400.0f;
    static constexpr float RETRACT_SPEED = 10.0f;
    static constexpr float ARRIVE_DIST   = 8.0f;

    float originX = 0.0f;
    float originY = 0.0f;

    bool update(const std::vector<Platform>& platforms,
                std::vector<Player>&      players,
                std::vector<Projectile>&  projectiles,
                float ts);

    void draw(SDL_Renderer* r) const;

    bool isLatched() const;
    void retract();

private:
    float distanceFromOrigin() const;

    bool pullOwnerToward(float hx, float hy, float ts);
};