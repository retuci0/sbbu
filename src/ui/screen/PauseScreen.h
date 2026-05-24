#pragma once

#include "../Screen.h"

#include <SDL2/SDL_ttf.h>


enum class PauseActionResult { 
    RESUME, 
    QUIT, 
    RESTART, 
    CHANGE_VOLUME 
};

class PauseScreen : public Screen {
public:
    PauseScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, int sw, int sh);
    void render(SDL_Renderer* renderer) override;
    bool isFinished() const { return finished; }
    PauseActionResult getResult() const { return result; }

private:
    TTF_Font* titleFont;
    TTF_Font* font;
    bool finished = false;
    PauseActionResult result = PauseActionResult::RESUME;
};