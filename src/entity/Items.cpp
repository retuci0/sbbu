#include "entity/Items.h"

#include "entity/Platform.h"
#include "entity/Player.h"

#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <algorithm>
#include <cmath>


Item::Item(const std::string& name, SDL_Rect spawnRect, SDL_Texture* tex, Mix_Chunk* sfx,
           float effectDuration, float respawnDelay)
    : Entity(spawnRect, 0.0f, 0.0f, tex)
    , name(name)
    , spawnRect(spawnRect)
    , sfx(sfx)
    , effectDuration(effectDuration)
    , respawnDelay(respawnDelay)
{}

void Item::draw(SDL_Renderer* r, float a) {
    if (consumer && isAffecting()) {
        drawEffect(r, a);
    }
    if (active) {
        Renderer::drawEntity(r, this, a);
    }
}

void Item::drawHitbox(SDL_Renderer* r, float a) const {
    if (!active) return;
    Renderer::drawHitbox(r, this, a);
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

void Item::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    // --- effect countdown ---
    if (effectTimer > 0.0f) {
        effectTimer -= ts;
        if (effectTimer <= 0.0f) {
            effectTimer = 0.0f;
            onEffectEnd();
        }
        if (consumer) {
            prevRect = rect;
            rect.x = consumer->rect.x;
            rect.y = consumer->rect.y;
        }
        return;
    }

    if (!active) return;

    Entity::update(entities, ts);

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

    for (const auto& e : entities) {
        if (Platform* p = dynamic_cast<Platform*>(e.get())) {
            const int prevBottom = prevRect.y + prevRect.h;
            if (prevBottom > p->rect.y) continue;
            if (!p->intersectsWith(*this)) continue;
            rect.y   = p->rect.y - rect.h;
            dy       = 0.0f;
            onGround = true;
            break;
        }

        // i completely made up this collision system lmfao
        if (Player* p = dynamic_cast<Player*>(e.get())) {
            if (intersectsWith(*p)) {
                float distX = (p->rect.x + p->rect.w / 2.0f) - (rect.x + rect.w / 2.0f);
                float distY = (p->rect.y + p->rect.h / 2.0f) - (rect.y + rect.h / 2.0f);
                dx = ((p->dx / p->character.stats.terminalVelocity) / 2.7f) * distX * (p->facing == Facing::LEFT ? 1 : -1);
                dy = ((p->dy / p->character.stats.terminalVelocity) / 2.7f) * distY;
            }
        }
    }
}

void Item::onPickup() {
    if (sfx) Mix_PlayChannel(-1, sfx, 0);
    active = false;
    if (effectDuration > 0.0f) {
        effectTimer = effectDuration;
    }
}

void Item::onEffectEnd() {
    consumer = nullptr;
}


/////////////////////////////////////////
/*            MUSHROOM ITEM            */
/////////////////////////////////////////

void MushroomItem::onPickup() {
    if (!consumer) return;

    // size up
    scalePlayer(consumer, 2.0f);

    // damage buff
    consumer->character.stats.damage           *= 2;
    consumer->character.stats.projectileDamage *= 2;

    Item::onPickup();
}

void MushroomItem::onEffectEnd() {
    if (!consumer) return;

    scalePlayer(consumer, 0.5f);

    consumer->character.stats.damage           /= 2;
    consumer->character.stats.projectileDamage /= 2;

    Item::onEffectEnd();
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

    Item::onEffectEnd();
}

void ShitItem::drawEffect(SDL_Renderer* r, float a) const {
    if (!consumer) return;

    int x  = static_cast<int>(consumer->rect.x + (consumer->rect.w - rect.w) / 2);
    int xo = static_cast<int>(consumer->prevRect.x + (consumer->prevRect.w - rect.w) / 2);
    int y  = consumer->rect.y - rect.h;
    int yo = consumer->prevRect.y - rect.h;
    SDL_Rect drawRect = interpolatedRect({ xo, yo, rect.w, rect.h }, { x, y, rect.w, rect.h }, a);
    
    Renderer::drawSprite(r, tex, &drawRect, false);
}


//////////////////////////////////////////////
/*               COCAINE ITEM               */
//////////////////////////////////////////////

void CocaineItem::onPickup() {
    if (!consumer) return;

    consumer->character.stats.velocity     *= 3;
    consumer->character.stats.acceleration *= 3;

    Item::onPickup();
}

void CocaineItem::onEffectEnd() {
    if (!consumer) return;

    consumer->character.stats.velocity     /= 3;
    consumer->character.stats.acceleration /= 3;

    Item::onEffectEnd();
}

void CocaineItem::drawEffect(SDL_Renderer* r, float a) const {
    if (!consumer) return;

    SDL_Rect drawRect = consumer->interpolatedRect(a);

    Renderer::drawSprite(r, overlay, &drawRect, consumer->facing == Facing::LEFT);
}


/////////////////////////////////////////////
/*               SPRING ITEM               */
/////////////////////////////////////////////

void SpringItem::onPickup() {
    if (!consumer) return;

    consumer->character.stats.jumpVelocity *= 1.5;

    Item::onPickup();
}

void SpringItem::onEffectEnd() {
    if (!consumer) return;
    
    consumer->character.stats.jumpVelocity /= 1.5;

    Item::onEffectEnd();
}

void SpringItem::drawEffect(SDL_Renderer* r, float a) const {
    if (!consumer) return;

    SDL_Rect drawRect = consumer->interpolatedRect(a);
    
    Renderer::drawSprite(r, overlay, &drawRect, consumer->facing == Facing::LEFT);
}


//////////////////////////////////////////////////
/*               ANGEL WINGS ITEM               */
//////////////////////////////////////////////////

void AngelWingsItem::onPickup() {
    if (!consumer) return;

    consumer->invulnerableTimer = effectDuration;

    Item::onPickup();
}

void AngelWingsItem::drawEffect(SDL_Renderer* r, float a) const {
    if (!consumer) return;

    SDL_Rect drawRect = consumer->interpolatedRect(a);

    Renderer::drawSprite(r, overlay, &drawRect, consumer->facing == Facing::LEFT);
}