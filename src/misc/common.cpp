#include "common.h"


void renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font || text.empty()) { return; }
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surf) { return; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void fillRect(SDL_Renderer* r, int x, int y, int w, int h, Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, red, green, blue, alpha);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void outlineRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    for (int i = 0; i < thickness; ++i) {
        SDL_Rect rect = {x + i, y + i, w - 2 * i, h - 2 * i};
        SDL_RenderDrawRect(r, &rect);
    }
}

SDL_Rect renderButton(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, int w, int h, SDL_Color bg, SDL_Color fg) {
    fillRect(r, x, y, w, h, bg.r, bg.g, bg.b, bg.a);
    int tw = 0, th = 0;
    TTF_SizeText(font, text.c_str(), &tw, &th);
    renderText(r, font, text, x + (w - tw) / 2, y + (h - th) / 2, fg);
    return {x, y, w, h};
}