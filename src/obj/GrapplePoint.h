#pragma once

#include "core/Resources.h"
#include "misc/Renderer.h"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


// inspo taken from Ultrakill
enum class GrapplePointType {
    GREEN,  // cancels momentum
    BLUE    // doesn't cancel momentum
};

class GrapplePoint {
public:
    GrapplePoint(GrapplePointType type, SDL_Rect rect)
        : type(type), rect(rect) 
    {
        tex = type == GrapplePointType::GREEN 
                ? Resources::get().getTexture("grapple_point_green")
                : Resources::get().getTexture("grapple_point_blue");
    }

    ~GrapplePoint() = default;

    GrapplePointType type;
    SDL_Rect rect;
    SDL_Texture* tex;

    void draw(SDL_Renderer* r, float /*a*/) {
        Renderer::drawSprite(r, tex, &rect, false);
    }

    void drawHitbox(SDL_Renderer* r, float /*a*/) {
        Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, BLUE, 2);
    }
};