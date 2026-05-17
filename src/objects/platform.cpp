#include "platform.h"
#include "../misc/common.h"

Platform::Platform(SDL_Texture* bigTex, SDL_Texture* smallTex, int x, int y, int w, int h, PlatformSize size) {
    if (size == PlatformSize::BIG) {
        image = bigTex;
        rect  = {x, y, w, h / 4};
    } else {
        image = smallTex;
        rect  = {x, y, w, h};
    }
}

void Platform::draw(SDL_Renderer* r) const {
    if (image) SDL_RenderCopy(r, image, nullptr, &rect);
}

void Platform::drawHitboxes(SDL_Renderer* r) const {
    outlineRect(r, rect.x, rect.y, rect.w, rect.h, BLUE);
}