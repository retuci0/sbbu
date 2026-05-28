#include "Platform.h"

#include "../Resources.h"
#include "../misc/Common.h"
#include "../misc/Renderer.h"


Platform::Platform(int x, int y, int w, int h, PlatformSize size) : size(size) {
    if (size == PlatformSize::BIG) {
        image = Resources::get().getTexture("platform_big");
        rect  = {x, y, w, h / 4};
    } else {
        image = Resources::get().getTexture("platform_small");
        rect  = {x, y, w, h};
    }
}

void Platform::draw(SDL_Renderer* r, float /*a*/) const {
    if (image) { 
        SDL_RenderCopy(r, image, nullptr, &rect); 
    }
}

void Platform::drawHitbox(SDL_Renderer* r, float /*a*/) const {
    Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, BLUE, 2);
}