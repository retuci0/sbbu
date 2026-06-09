#include "Projectile.h"

#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"
#include "obj/Entity.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


Projectile::Projectile(int x, int y, Facing dir, Player* owner)
    : owner(owner)
{
    facing = dir;
    tex = Resources::get().getTexture("projectile");
    rect = { x, y, static_cast<int>(64 * owner->scale), static_cast<int>(64 * owner->scale) };
}

void Projectile::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    if (parryFlashTimer > 0) parryFlashTimer -= ts;
    if (parryFreezeTimer > 0) {
        parryFreezeTimer -= ts;
        return;
    }
    
    if (facing == Facing::LEFT) { 
        dx = -static_cast<int>(velocity * ts); 
    } else { 
        dx = +static_cast<int>(velocity * ts); 
    }

    Entity::update(entities, ts);
}

void Projectile::parry(Player* newOwner) {
    if (!newOwner || parryFreezeTimer > 0) return;

    owner = newOwner;
    facing = newOwner->facing;
    velocity *= 2.0f;
    parryFreezeTimer = PARRY_FREEZE_DURATION;
    parryFlashTimer = PARRY_FLASH_DURATION;
}

void Projectile::draw(SDL_Renderer* r, float a) {
    Renderer::drawEntity(r, this, a);
    if (parryFlashTimer > 0.0f && (static_cast<int>(parryFlashTimer) / 2) % 2 == 0) {
        Renderer::outlineRect(r, rect.x - 4, rect.y - 4, rect.w + 8, rect.h + 8, WHITE, 4);
    }
}

void Projectile::drawHitbox(SDL_Renderer* r, float a) const {
    Renderer::drawHitbox(r, this, a);
}
