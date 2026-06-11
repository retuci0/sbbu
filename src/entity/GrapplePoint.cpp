#include "entity/GrapplePoint.h"

#include "entity/Entity.h"
#include "entity/Player.h"
#include "entity/Grapple.h"

#include "misc/Renderer.h"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>

#include <memory>
#include <vector>

GrapplePoint::GrapplePoint(
        GrapplePointType type,
        SDL_Rect rect,
        std::function<void(std::vector<std::unique_ptr<Entity>>&)> action,
        float duration)
    : Entity(rect)
    , type(type)
    , action(action)
    , duration(duration)
{
    switch (type) {
        case GrapplePointType::GREEN:
            tex = Resources::get().getTexture("grapple_point_green");
            break;
        case GrapplePointType::BLUE:
            tex = Resources::get().getTexture("grapple_point_blue");
            break;
        case GrapplePointType::YELLOW:
            tex = Resources::get().getTexture("grapple_point_yellow_active");
            inactiveTex = Resources::get().getTexture("grapple_point_yellow_inactive");
            break;
    }
}

void GrapplePoint::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    if (cooldown > 0.0f) {
        cooldown -= ts;
        if (cooldown <= 0.0f) {
            active = true;
        }
    }

    if (type != GrapplePointType::YELLOW) {
        return;
    }

    if (!active || cooldown > 0.0f) return;

    for (auto& e : entities) {
        Player* player = dynamic_cast<Player*>(e.get());
        if (player && player->grapple && player->grapple->targetPoint == this) {
            trigger(entities);
            break;  // only trigger once per latch
        }
    }
}

void GrapplePoint::trigger(std::vector<std::unique_ptr<Entity>>& entities) {
    if (!active || cooldown > 0.0f) return;
    if (action) {
        action(entities);
    }
    active = false;
    cooldown = duration;
}


void GrapplePoint::draw(SDL_Renderer* r, float a) {
    if (type == GrapplePointType::YELLOW) {
        Renderer::drawSprite(
            r,
            active ? tex : inactiveTex,
            &rect,
            false
        );
        return;
    }

    Renderer::drawEntity(r, this, -1);
}

void GrapplePoint::drawHitbox(SDL_Renderer* r, float a) const {
    Renderer::drawHitbox(r, this, -1);
}