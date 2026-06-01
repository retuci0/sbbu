#include "misc/Renderer.h"

#include "misc/Color.h"
#include "misc/Common.h"

#include <SDL2/SDL_render.h>


static constexpr int CHECKER_SIZE = 16;

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

int Renderer::fillCircle(SDL_Renderer* renderer, int x, int y, int r, Color color) {
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = r;
    d = r - 1;
    status = 0;

    while (offsety >= offsetx) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx, x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety, x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety, x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx, x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx += 1;
        } else if (d < 2 * (r - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        } else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}

bool Renderer::drawSprite(SDL_Renderer *renderer, SDL_Texture *tex, const SDL_Rect* rect, const bool flipH, const double angle) {
    if (!renderer) return false;
    if (!rect) return false;

    // missing texture, draw the Garry's Mod missing texture texture
    if (!tex) {
        for (int y = 0; y < rect->h; y += CHECKER_SIZE) {
            int block_h = (y + CHECKER_SIZE > rect->h) ? (rect->h - y) : CHECKER_SIZE;
            for (int x = 0; x < rect->w; x += CHECKER_SIZE) {
                int block_w = (x + CHECKER_SIZE > rect->w) ? (rect->w - x) : CHECKER_SIZE;
                Color c = ((x / CHECKER_SIZE) + (y / CHECKER_SIZE)) % 2 == 0
                                    ? MAGENTA : BLACK;
                fillRect(renderer, rect->x + x, rect->y + y, block_w, block_h, c);
            }
        }
        return false;
    }

    // draw the sprite to the renderer
    return SDL_RenderCopyEx(
        renderer, 
        tex, 
        nullptr, 
        rect, 
        angle, 
        nullptr, 
        flipH 
            ? SDL_FLIP_HORIZONTAL 
            : SDL_FLIP_NONE
    ) == 0;
}

void Renderer::renderArrow(SDL_Renderer* r, int cx, int cy, Facing dir) {
    // three vertices of a triangle
    int tip = dir == Facing::LEFT ? cx - 18 : cx + 18;
    int base = dir == Facing::LEFT ? cx + 10 : cx - 10;
    SDL_Vertex verts[3] = {
        { { static_cast<float>(tip),    static_cast<float>(cy)      }, {255,255,255,220}, {0,0} },
        { { static_cast<float>(base),   static_cast<float>(cy - 14) }, {255,255,255,220}, {0,0} },
        { { static_cast<float>(base),   static_cast<float>(cy + 14) }, {255,255,255,220}, {0,0} },
    };
    SDL_RenderGeometry(r, nullptr, verts, 3, nullptr, 0);
}