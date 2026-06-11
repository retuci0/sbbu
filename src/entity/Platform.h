#pragma once

#include "entity/Entity.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>


enum class PlatformSize {
    BIG,
    SMALL
};

class Platform : public Entity {
public:
    PlatformSize size;

    Platform(int x, int y, int w, int h, PlatformSize size);

    void draw(SDL_Renderer* r, float a) override;
    void drawHitbox(SDL_Renderer* r, float a) const override;

    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;

    bool intersectsWith(const Entity& other) const override;

    bool active = true;
    float inactiveTimer = 0.0f;

    EntityType getType() const override {
        return EntityType::PLATFORM;
    }

private:
    SDL_Texture* inactiveTex;
};