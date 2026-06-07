#include "obj/Items.h"

#include "obj/Platform.h"
#include "obj/Player.h"

#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <algorithm>
#include <cmath>


Item::Item(const std::string& name, SDL_Rect spawnRect, SDL_Texture* tex, Mix_Chunk* sfx,
           float effectDuration, float respawnDelay)
    : name(name)
    , rect(spawnRect)
    , prevRect(spawnRect)
    , spawnRect(spawnRect)
    , tex(tex)
    , sfx(sfx)
    , effectDuration(effectDuration)
    , respawnDelay(respawnDelay)
{}

void Item::draw(SDL_Renderer* r, float a) const {
    if (!active) return;
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    Renderer::drawSprite(r, tex, &drawRect, false);
}

void Item::drawHitbox(SDL_Renderer* r, float a) const {
    if (!active) return;
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    Renderer::outlineRect(r, drawRect.x, drawRect.y, drawRect.w, drawRect.h, LIME, 2);
}

void Item::takeDamage(int damage, Facing side, float kbScale) {
    if (!active || consumer != nullptr) return;

    hp -= damage;

    float force = damage * 0.8f * kbScale;
    dx = (side == Facing::LEFT) ? -force : force;

    if (hp <= 0) {
        hp = 0;
    }
}

void Item::update(std::vector<Player*>& players,
                  const std::vector<Platform>& platforms,
                  const std::vector<Projectile>& /*projectiles*/,
                  float ts)
{
    if (!active) return;

    prevRect = rect;

    dx *= KB_FRICTION;
    if (std::abs(dx) < 0.1f) dx = 0.0f;
    rect.x += static_cast<int>(dx * ts);

    // --- gravity ---
    if (!onGround) {
        dy = std::min(dy + GRAVITY * ts, TERMINAL_VEL);
    } else {
        dy = 0.0f;
    }
    rect.y += static_cast<int>(dy * ts);
    onGround = false;

    for (const auto& p : platforms) {
        const int prevBottom = prevRect.y + prevRect.h;
        if (prevBottom > p.rect.y) continue;
        if (!SDL_HasIntersection(&rect, &p.rect)) continue;
        rect.y   = p.rect.y - rect.h;
        dy       = 0.0f;
        onGround = true;
        break;
    }

    // --- effect countdown ---
    if (effectTimer > 0.0f) {
        effectTimer -= ts;
        if (effectTimer <= 0.0f) {
            effectTimer = 0.0f;
            onEffectEnd();
        }
        return;
    }
}

void Item::onPickup() {
    if (sfx) Mix_PlayChannel(-1, sfx, 0);
    active = false;
    if (effectDuration > 0.0f) {
        effectTimer = effectDuration;
    }
}



/////////////////////////////////////////
/*            MUSHROOM ITEM            */
/////////////////////////////////////////

void MushroomItem::onPickup() {
    if (!consumer) return;

    // save current stats to restore them later
    prevDmg     = consumer->character->stats.damage;
    prevProjDmg = consumer->character->stats.projectileDamage;

    // size up
    scalePlayer(consumer, 2.0f);
    consumer->dontResize = true;

    //damage buff
    consumer->damage     *= 2;
    consumer->projDamage *= 2;

    Item::onPickup();
}

void MushroomItem::onEffectEnd() {
    if (!consumer) return;

    scalePlayer(consumer, 0.5f);
    consumer->dontResize = false;

    consumer->damage     = prevDmg;
    consumer->projDamage = prevProjDmg;

    consumer = nullptr;
}

void MushroomItem::scalePlayer(Player* player, float k) {
    player->scale *= k;

    int newW = static_cast<int>(std::round(player->rect.w * k));
    int newH = static_cast<int>(std::round(player->rect.h * k));

    // keep feet planted
    player->rect.x -= newW - player->rect.w;
    player->rect.y -= newH - player->rect.h;
    player->rect.w  = newW;
    player->rect.h  = newH;
}


///////////////////////////////////////////
/*               SHIT ITEM               */
///////////////////////////////////////////

void ShitItem::onPickup() {
    if (!consumer) return;
    consumer->shitAuraTimer = effectDuration;
    Item::onPickup();
}

void ShitItem::onEffectEnd() {
    if (!consumer) return;

    consumer->shitAuraTimer = 0.0f;

    consumer = nullptr;
}