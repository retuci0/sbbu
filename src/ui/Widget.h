#pragma once

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


class Widget {
public:
    Widget(int x, int y, int w, int h)
    : rect({ x, y, w, h }) {}

    virtual ~Widget() = default;
    virtual void draw(SDL_Renderer* renderer, TTF_Font* font) = 0;
    virtual bool handle(const SDL_Event& event) = 0;

    SDL_Rect getRect() const { return rect; }

protected:
    SDL_Rect rect;
};