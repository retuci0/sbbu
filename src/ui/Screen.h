#pragma once

#include "ui/Button.h"

#include <SDL2/SDL.h>


class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(const SDL_Event& e) { for (auto& b : buttons) b.handle(e); }
    virtual void update() {}
    virtual void render(SDL_Renderer* r) = 0;

protected:
    std::vector<Button> buttons;
};