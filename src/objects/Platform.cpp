#include "Platform.h"

#include "../misc/Common.h"
#include "../misc/Renderer.h"


Platform::Platform(SDL_Texture* bigTex, SDL_Texture* smallTex, int x, int y, int w, int h, PlatformSize size) : size(size) {
    if (size == PlatformSize::BIG) {
        image = bigTex;
        rect  = {x, y, w, h / 4};
    } else {
        image = smallTex;
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