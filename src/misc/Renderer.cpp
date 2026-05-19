#include "Renderer.h"

#include "Color.h"


void Renderer::renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, Color c) {
    // return if font hasn't been loaded or string is empty
    if (!font || text.empty()) return;
    // get SDL_Surface of text
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), c.toSdlColor());
    if (!surf) return;
    // get SDL_Texture of surface
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    // free the surface
    SDL_FreeSurface(surf);
}

void Renderer::fillRect(SDL_Renderer* r, int x, int y, int w, int h, Color c) {
    // enable blend
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    // set draw color
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    // draw filled rect
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void Renderer::outlineRect(SDL_Renderer* r, int x, int y, int w, int h, Color c, int thickness) {
    // set draw color
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    // draw rect outline
    for (int i = 0; i < thickness; ++i) {
        SDL_Rect rect = {x + i, y + i, w - 2 * i, h - 2 * i};
        SDL_RenderDrawRect(r, &rect);
    }
}

SDL_Rect Renderer::renderButton(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, int w, int h, Color bg, Color fg) {
    // draw background
    fillRect(r, x, y, w, h, bg);
    // draw text
    int tw = 0, th = 0;
    TTF_SizeText(font, text.c_str(), &tw, &th);
    renderText(r, font, text, x + (w - tw) / 2, y + (h - th) / 2, fg);
    // return SDL_Rect for hit-checks
    return {x, y, w, h};
}