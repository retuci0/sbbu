#include "entity/Platform.h"

#include "core/Resources.h"
#include "misc/Renderer.h"


Platform::Platform(int x, int y, int w, int h, PlatformSize size) 
    : Entity({x, y, w, h}), size(size) 
{
    if (size == PlatformSize::BIG) {
        tex = Resources::get().getTexture("platform_big");
    } else {
        tex = Resources::get().getTexture("platform_small");
    }
}

void Platform::draw(SDL_Renderer* r, float /*a*/) {
    Renderer::drawEntity(r, this, -1);
}

void Platform::drawHitbox(SDL_Renderer* r, float /*a*/) const {
    Renderer::drawHitbox(r, this, -1);
}