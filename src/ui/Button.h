#pragma once

#include "misc/Color.h"
#include "misc/Common.h"
#include "misc/Renderer.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <functional>
#include <string>

class Button {
public:
    Button(int x, int y, int w, int h, const std::string& label, Color bg, Color fg, std::function<void()> action) 
    : rect({ x, y, w, h}), label(label), bg(bg), fg(fg), action(action) {}

    void draw(SDL_Renderer* renderer, TTF_Font* font) {
        Renderer::renderButton(renderer, font, label, rect.x, rect.y, rect.w, rect.h, bg, fg);
    }

    bool handle(const SDL_Event& event) {
        if (event.type != SDL_MOUSEBUTTONDOWN) return false;
        if (!pointInRect(event.button.x, event.button.y, rect)) return false;
        if (action) action();
        return true;
    }

private:
    std::string label;  
    std::function<void()> action;
    SDL_Rect rect;
    Color bg, fg;
};