#include "Projectile.h"

#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


Projectile::Projectile(int x, int y, Facing dir, Player* owner)
    : direction(dir), owner(owner)
{
    img = Resources::get().getTexture("projectile");
    rect = {x, y, static_cast<int>(64 * owner->scale), static_cast<int>(64 * owner->scale)};
}

void Projectile::update(float ts) {
    prevRect = rect;
    if (parryFlashTimer > 0) parryFlashTimer -= ts;
    if (parryFreezeTimer > 0) {
        parryFreezeTimer -= ts;
        return;
    }

    if (direction == Facing::LEFT) { 
        rect.x -= static_cast<int>(velocity * ts); 
    } else { 
        rect.x += static_cast<int>(velocity * ts); 
    }
}

void Projectile::parry(Player* newOwner) {
    if (!newOwner || parryFreezeTimer > 0) return;

    owner = newOwner;
    direction = newOwner->facing;
    velocity *= 2.0f;
    parryFreezeTimer = PARRY_FREEZE_DURATION;
    parryFlashTimer = PARRY_FLASH_DURATION;
}

void Projectile::draw(SDL_Renderer* r, float a) {
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    Renderer::drawSprite(r, img, &drawRect, direction == Facing::LEFT);
    if (parryFlashTimer > 0.0f && (static_cast<int>(parryFlashTimer) / 2) % 2 == 0) {
        Renderer::outlineRect(r, drawRect.x - 4, drawRect.y - 4, drawRect.w + 8, drawRect.h + 8, WHITE, 4);
    }
}

void Projectile::drawHitbox(SDL_Renderer* r, float a) const {
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    Renderer::outlineRect(r, drawRect.x, drawRect.y, drawRect.w, drawRect.h, LIME, 2);
}
