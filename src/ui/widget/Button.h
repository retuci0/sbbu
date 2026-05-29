#pragma once

#include "../Widget.h"

#include "../../Resources.h"
#include "../../misc/Color.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <functional>
#include <string>


class Button : public Widget {
public:
Button(int x, int y, int w, int h, const std::string& label, Color bg, Color fg, std::function<void()> action, SDL_Texture* texture = nullptr)
    : Widget(x, y, w, h), label(label), bg(bg), fg(fg), action(action), texture(texture) 
    {
        clickSound = Resources::get().getSound("click");
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) override {
        Renderer::renderButton(renderer, font, label, rect.x, rect.y, rect.w, rect.h, 
            bg == WHITE 
                ? (hovered ? bg.darker() : bg)
                : hovered ? bg.brighter() : bg,
            fg
        );
        if (texture) {
            Renderer::drawSprite(renderer, texture, &rect, false);
        }
    }

    bool handle(const SDL_Event& e) override {
        if (!pointInRect(e.button.x, e.button.y, rect)) {
            hovered = false;
            return false;
        } else {
            hovered = true;
        }
        if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) {
            return false;
        }
        activate();
        return true;
    }

    void activate() {
        if (action) action();
        if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
    }

    void setColors(Color bg, Color fg) {
        this->bg = bg;
        this->fg = fg;
    }

private:
    SDL_Texture* texture;
    std::string label;
    std::function<void()> action;
    Color bg, fg;
    bool hovered;
    Mix_Chunk* clickSound = nullptr;
};