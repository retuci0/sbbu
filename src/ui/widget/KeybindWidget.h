#pragma once

#include "../Widget.h"

#include "../../misc/Color.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


class KeybindWidget : public Widget {
public:
    KeybindWidget(int x, int y, int w, int h, const std::string& actionName, SDL_KeyCode& binding)
    : Widget(x, y, w, h), actionName(actionName), binding(binding) {}

    bool handle(const SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (pointInRect(e.button.x, e.button.y, rect))
                listening = true;
            return false;
        }
        if (listening && e.type == SDL_KEYDOWN) {
            SDL_Keycode k = e.key.keysym.sym;
            if (k != SDLK_ESCAPE) binding = static_cast<SDL_KeyCode>(k);
            listening = false;
        }
        return true;
    }

    void draw(SDL_Renderer* r, TTF_Font* font) override {
        Color bg = listening ? Color{80, 80, 180, 255} : Color{50, 50, 50, 255};
        Renderer::fillRect(r, rect.x, rect.y, rect.w, rect.h, bg);
        Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, {100, 100, 100, 255}, 1);

        // action name on the left
        Renderer::renderText(r, font, actionName, rect.x + 12, rect.y + 10, WHITE);

        // key name on the right
        std::string keyLabel = listening ? "press a key..." : SDL_GetKeyName(binding);
        int tw, th;
        TTF_SizeText(font, keyLabel.c_str(), &tw, &th);
        Color keyColor = listening ? Color{255, 220, 80, 255} : Color{180, 220, 180, 255};
        Renderer::renderText(r, font, keyLabel, rect.x + rect.w - tw - 12, rect.y + 10, keyColor);
    }

    bool isListening() const { return listening; }

private:
    std::string actionName;
    SDL_KeyCode& binding;  // non-owning
    bool listening = false;
};