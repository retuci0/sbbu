#pragma once

#include "entity/Entity.h"

#include "entity/GrapplePoint.h"

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

class Grapple : public Entity {
public:
    Grapple(Player& player, int startX, int startY, float dx, float dy);

    Player&   owner;
    GrappleState state = GrappleState::FLYING;

    Facing direction = Facing::RIGHT;

    float playerDx0 = 0, playerDy0 = 0;

    Player*         targetPlayer     = nullptr;
    Projectile*     targetProjectile = nullptr;
    GrapplePoint*   targetPoint      = nullptr;
    Item*           targetItem       = nullptr;

    bool isAlive() const { return alive; }

    static constexpr float TRAVEL_SPEED  = 24.0f;
    static constexpr float PULL_FORCE    = 12.0f;
    static constexpr float MAX_RANGE     = 400.0f;
    static constexpr float RETRACT_SPEED = 10.0f;
    static constexpr float ARRIVE_DIST   = 64.0f;

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;

    void draw(SDL_Renderer* r, float a) override;
    void drawHitbox(SDL_Renderer* r, float a) const override;
    
    bool isLatched() const;
    void retract();

    EntityType getType() const override {
        return EntityType::MISC;
    }

private:
    float distanceFromOrigin() const;

    bool pullOwnerToward(float hx, float hy, float ts);
    bool pullItemTowardOwner(Item* item, float ts);

    bool alive = true;
    bool velocitySnapshotted = false;
};