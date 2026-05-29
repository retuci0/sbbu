#include "Shockwave.h"

#include "core/Resources.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_render.h>


void ShockwaveRect::update(float ts)  {
    if (!active) return;
    prevRect = rect;
    rect.x += static_cast<int>(dx * ts);
    rect.y += static_cast<int>(dy * ts);
    // deactivate once the rect leaves the screen
    if (rect.x + rect.w < 0 || rect.x > SW) active = false;
}

Shockwave::Shockwave(int spawnX, int spawnY, Player* owner)
    : owner(owner),
    rects(ShockwaveRect(spawnX, spawnY, 32, 32, owner),
          ShockwaveRect(spawnX, spawnY, 32, 32, owner)
) {
    img = Resources::get().getTexture("shockwave");
    rects.first.dx  = -7.0f;   // travels left
    rects.second.dx =  7.0f;   // travels right
}

void Shockwave::update(float ts) {
    rects.first.update(ts);
    rects.second.update(ts);
}

void Shockwave::draw(SDL_Renderer* r, float a) {
    SDL_Rect drawRect1 = interpolatedRect(rects.first.prevRect, rects.first.rect, a);
    SDL_Rect drawRect2 = interpolatedRect(rects.second.prevRect, rects.second.rect, a);
    Renderer::drawSprite(r, img, &drawRect1, false);
    Renderer::drawSprite(r, img, &drawRect2, false);
}

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

void Shockwave::drawHitboxes(SDL_Renderer* renderer, float a) {
    SDL_Rect drawRect1 = interpolatedRect(rects.first.prevRect, rects.first.rect, a);
    SDL_Rect drawRect2 = interpolatedRect(rects.second.prevRect, rects.second.rect, a);
    Renderer::outlineRect(renderer, drawRect1.x, drawRect1.y, drawRect1.w, drawRect1.h, GREEN, 2);
    Renderer::outlineRect(renderer, drawRect2.x, drawRect2.y, drawRect2.w, drawRect2.h, GREEN, 2);
}