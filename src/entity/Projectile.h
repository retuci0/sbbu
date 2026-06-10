#pragma once

#include "entity/Entity.h"

#include "entity/Player.h"

#include "misc/Common.h"

#include <SDL2/SDL.h>


class Projectile : public Entity {
public:
    float velocity = 19.0f;
    float parryFreezeTimer = 0.0f;
    float parryFlashTimer = 0.0f;
    
    Player* owner = nullptr;  // non-owning

    Projectile(int x, int y, Facing dir, Player* owner);

    static constexpr int PARRY_FREEZE_DURATION = 12;
    static constexpr int PARRY_FLASH_DURATION = 10;

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
    void parry(Player* newOwner);
    void draw(SDL_Renderer* r, float a) override;
    void drawHitbox(SDL_Renderer* r, float a) const override;

    EntityType getType() const override {
        return EntityType::MISC;
    }
};
