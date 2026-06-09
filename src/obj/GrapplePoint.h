#pragma once

#include "obj/Entity.h"

#include "core/Resources.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


// inspo taken from Ultrakill
enum class GrapplePointType {
    GREEN,  // cancels momentum
    BLUE    // doesn't cancel momentum
};

class GrapplePoint : public Entity {
public:
    GrapplePoint(GrapplePointType type, SDL_Rect rect)
        : Entity(rect), type(type) 
    {
        tex = type == GrapplePointType::GREEN 
                ? Resources::get().getTexture("grapple_point_green")
                : Resources::get().getTexture("grapple_point_blue");
    }

    ~GrapplePoint() = default;
    GrapplePointType type;

    void draw(SDL_Renderer* r, float /*a*/) override {
        Renderer::drawSprite(r, tex, &rect, false);
    }

    void drawHitbox(SDL_Renderer* r, float /*a*/) const override {
       Renderer::drawHitbox(r, this, -1);
    }

    EntityType getType() const override {
        return EntityType::PLATFORM;
    }
};