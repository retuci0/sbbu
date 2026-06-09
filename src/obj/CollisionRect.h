#pragma once

#include "obj/Entity.h"
#include "obj/Player.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


class CollisionRect : public Entity {
public:
    CollisionRect(int x, int y, int w, int h, Player* owner, int durationFrames = 5);

    Player* owner;
    float lifetime = 0.0f;
    float damageScale = 1.0f;
    float kbScale     = 1.0f;

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
    bool isAlive() const { return lifetime > 0; }
    void drawHitbox(SDL_Renderer* r, float a);
};