#include "Projectile.h"

#include "../misc/Common.h"
#include "../misc/Renderer.h"
#include <SDL2/SDL_render.h>


Projectile::Projectile(SDL_Texture* img, int x, int y, Facing dir, Player* owner)
    : direction(dir), img(img), owner(owner)
{
    rect = {x, y, 64, 64};
}

void Projectile::move() {
    if (parryFlashTimer > 0) --parryFlashTimer;
    if (parryFreezeTimer > 0) {
        --parryFreezeTimer;
        return;
    }

    if (direction == Facing::LEFT) { 
        rect.x -= static_cast<int>(velocity); 
    } else { 
        rect.x += static_cast<int>(velocity); 
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

void Projectile::draw(SDL_Renderer* r) {
    if (img) { 
        SDL_RenderCopyEx(r, img, nullptr, &rect, 0, 0, direction == Facing::LEFT 
                        ? SDL_RendererFlip::SDL_FLIP_HORIZONTAL
                        : SDL_RendererFlip::SDL_FLIP_NONE
        ); 
    }
    if (parryFlashTimer > 0 && (parryFlashTimer / 2) % 2 == 0) {
        Renderer::outlineRect(r, rect.x - 4, rect.y - 4, rect.w + 8, rect.h + 8, WHITE, 4);
    }
}

void Projectile::drawHitbox(SDL_Renderer* r) const {
    Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, GREEN, 2);
}
