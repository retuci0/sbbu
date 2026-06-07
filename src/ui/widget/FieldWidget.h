#pragma once

#include "misc/Color.h"
#include "misc/Common.h"
#include "misc/Renderer.h"
#include "ui/Widget.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <string>


class FieldWidget : public Widget {
public:
    FieldWidget(int x, int y, int w, int h,  TTF_Font* font, const std::string& initialText = "")
    : Widget(x, y, w, h), text(initialText), font(font) {}

    bool handle(const SDL_Event& event) override {
        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (isHovered(event.button.x, event.button.y) && event.button.button == SDL_BUTTON_LEFT) {
                    active = true;
                } else {
                    active = false;
                }
                return true;
            case SDL_TEXTINPUT:
                if (active) {
                    int w;
                    TTF_SizeText(font, (text + event.text.text).c_str(), &w, nullptr);
                    if (w <= rect.w) {
                        text += event.text.text;
                    }
                    return true;
                }
                return false;
            case SDL_KEYDOWN:
                if (!active) break;
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active = false;
                    return true;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (!text.empty()) text.pop_back();
                    return true;
                }
        }
        return false;
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) override {
        Color c = active ? Color{ 100, 100, 180, 255 } : Color{ 60, 60, 60, 255 };
        Renderer::fillRect(renderer, rect.x, rect.y, rect.w, rect.h, c);
        int w;
        TTF_SizeText(font, (text + "_").c_str(), &w, nullptr);
        std::string t = active && w <= rect.w ? text + "_" : text;
        Renderer::renderText(renderer, font, t, rect.x + 2, rect.y + 2, WHITE);
    }

    const std::string& getText() const { return text; }

private:
    std::string text;
    TTF_Font* font;
    bool active = false;
};