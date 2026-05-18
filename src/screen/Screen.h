#pragma once

#include <SDL2/SDL.h>


class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(const SDL_Event& e) = 0;
    virtual void update() = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
};