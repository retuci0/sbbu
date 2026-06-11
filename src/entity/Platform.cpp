#include "entity/Platform.h"

#include "core/Resources.h"
#include "misc/Renderer.h"
#include <memory>
#include <string>
#include <vector>


Platform::Platform(int x, int y, int w, int h, PlatformSize size) 
    : Entity({x, y, w, h}), size(size) 
{
    if (size == PlatformSize::BIG) {
        tex = Resources::get().getTexture("platform_big_active");
        inactiveTex = Resources::get().getTexture("platform_big_inactive");
    } else {
        tex = Resources::get().getTexture("platform_small_active");
        inactiveTex = Resources::get().getTexture("platform_small_inactive");
    }
}

void Platform::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    if (inactiveTimer > 0.0f) {
        inactiveTimer -= ts;
    }

    if (inactiveTimer <= 0.0f) active = true;
    else                       active = false;

    SDL_Log(std::to_string(inactiveTimer).c_str());
}

void Platform::draw(SDL_Renderer* r, float /*a*/) {
    if (active) {
        Renderer::drawSprite(r, tex, &rect, false);
    } else {
        Renderer::drawSprite(r, inactiveTex, &rect, false);
    }
}

void Platform::drawHitbox(SDL_Renderer* r, float /*a*/) const {
    if (active) Renderer::drawHitbox(r, this, -1);
}

bool Platform::intersectsWith(const Entity& other) const {
    return Entity::intersectsWith(other) && active;
}