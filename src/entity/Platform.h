#pragma once

#include "entity/Entity.h"

#include <SDL2/SDL.h>


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

    EntityType getType() const override {
        return EntityType::PLATFORM;
    }
};