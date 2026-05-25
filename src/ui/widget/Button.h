#pragma once

#include "../Widget.h"

#include "../../misc/Color.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_ttf.h>

#include <functional>
#include <string>


class Button : public Widget {
public:
    Button(int x, int y, int w, int h, const std::string& label, Color bg, Color fg, std::function<void()> action)
    : Widget(x, y, w, h), label(label), bg(bg), fg(fg), action(action) {}

    void draw(SDL_Renderer* renderer, TTF_Font* font) override {
        Renderer::renderButton(renderer, font, label, rect.x, rect.y, rect.w, rect.h, bg, fg);
    }

    bool handle(const SDL_Event& e) override {
        if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return false;
        if (!pointInRect(e.button.x, e.button.y, rect)) return false;
        if (action) action();
        return true;
    }

private:
    std::string label;
    std::function<void()> action;
    Color bg, fg;
};