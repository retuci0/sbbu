#pragma once

#include "entity/Entity.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <functional>
#include <memory>
#include <vector>


// inspo taken from Ultrakill
enum class GrapplePointType {
    GREEN,  // cancels momentum
    BLUE,   // doesn't cancel momentum
    YELLOW  // triggers something
};

class GrapplePoint : public Entity {
public:
    GrapplePoint(
        GrapplePointType type,
        SDL_Rect rect,
        std::function<void(std::vector<std::unique_ptr<Entity>>&)> action = nullptr,
        float duration = 0.0f
    );

    GrapplePointType type;

    bool active = true;
    float duration = 0.0f;
    float cooldown = 0.0f;

    std::function<void(std::vector<std::unique_ptr<Entity>>&)> action;

    void draw(SDL_Renderer* r, float a) override;
    void drawHitbox(SDL_Renderer* r, float a) const override;
    void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;

    void trigger(std::vector<std::unique_ptr<Entity>>& entities);

private:
    SDL_Texture* inactiveTex = nullptr;
};