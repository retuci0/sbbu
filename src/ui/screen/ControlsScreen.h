#pragma once

#include "../Screen.h"

#include "../../Options.h"

#include <SDL2/SDL_ttf.h>


class ControlsScreen : public Screen {
public:
    ControlsScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, Options& options);
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;
    bool isFinished() const { return finished; }
    
private:
    TTF_Font* titleFont;
    TTF_Font* font;
    bool finished = false;
    int selectedRow = 0, selectedColumn = 0;
};