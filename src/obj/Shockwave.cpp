#include "Shockwave.h"

#include "core/Resources.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_render.h>


void ShockwaveRect::update(std::vector<std::unique_ptr<Entity>>& entities, float ts)  {
    if (!active) return;
    Entity::update(entities, ts);
    // deactivate once the rect leaves the screen
    if (rect.x + rect.w < 0 || rect.x > SW) active = false;
}

Shockwave::Shockwave(int spawnX, int spawnY, Player* owner)
    : owner(owner),
    rects(ShockwaveRect(spawnX, spawnY, SHOCKWAVE_SIZE * owner->scale, SHOCKWAVE_SIZE * owner->scale, owner),
          ShockwaveRect(spawnX, spawnY, SHOCKWAVE_SIZE * owner->scale, SHOCKWAVE_SIZE * owner->scale, owner)
) {
    tex = Resources::get().getTexture("shockwave");
    rects.first.dx  = -SHOCKWAVE_SPEED;   // travels left
    rects.second.dx =  SHOCKWAVE_SPEED;   // travels right
}

void Shockwave::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    rects.first.update(entities, ts);
    rects.second.update(entities, ts);
}

void Shockwave::draw(SDL_Renderer* r, float a) {
    Renderer::drawEntity(r, &rects.first, a);
    Renderer::drawEntity(r, &rects.second, a);
}

// replace with raw pointer to match rest of codebase perhaps?
std::optional<Facing> Shockwave::checkCollision(const Player& player) {
    if (rects.first.active  && SDL_HasIntersection(&player.rect, &rects.first.rect)) {
        rects.first.active = false;
        return Facing::LEFT;
    }
    if (rects.second.active && SDL_HasIntersection(&player.rect, &rects.second.rect)) {
        rects.second.active = false;
        return Facing::RIGHT;
    }
    return std::nullopt;
}

void Shockwave::drawHitbox(SDL_Renderer* r, float a) const {
    Renderer::drawHitbox(r, &rects.first, a);
    Renderer::drawHitbox(r, &rects.second, a);
}