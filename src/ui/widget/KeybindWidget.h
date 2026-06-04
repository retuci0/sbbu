#pragma once

#include "ui/Widget.h"

#include "core/InputHandler.h"

#include "misc/Color.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_ttf.h>

#include <string>


class KeybindWidget : public Widget {
public:
    KeybindWidget(int x, int y, int w, int h,
                  const std::string& actionName, SDL_KeyCode& binding,
                  InputHandler& input,
                  SDL_GameControllerButton controllerButton = SDL_CONTROLLER_BUTTON_INVALID)
        : Widget(x, y, w, h), actionName(actionName), binding(binding), input(input), controllerButton(controllerButton) {}

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

        // if a controller is connected and this action has a controller binding, show that instead
        bool showController = !listening
            && controllerButton != SDL_CONTROLLER_BUTTON_INVALID
            && input.isControllerConnected(0);

        std::string keyLabel;
        Color keyColor;
        if (listening) {
            keyLabel = "press a key...";
            keyColor = {255, 220, 80, 255};
        } else if (showController) {
            keyLabel = getControllerButtonName(controllerButton);
            keyColor = {220, 180, 255, 255};  // soft purple to distinguish controller labels
        } else {
            keyLabel = SDL_GetKeyName(binding);
            keyColor = {180, 220, 180, 255};
        }

        int tw, th;
        TTF_SizeText(font, keyLabel.c_str(), &tw, &th);
        Renderer::renderText(r, font, keyLabel, rect.x + rect.w - tw - 12, rect.y + 10, keyColor);
    }

    bool isListening() const { return listening; }
    void startListening() { listening = true; }

private:
    std::string actionName;
    SDL_KeyCode& binding;           // non-owning
    InputHandler& input;            // non-owning
    SDL_GameControllerButton controllerButton;
    bool listening = false;
};