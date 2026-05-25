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
    PauseScreen(SDL_Renderer* renderer, int sw, int sh, TTF_Font* titleFont);

    void render(SDL_Renderer* renderer, TTF_Font* font) override;

    bool isFinished() const { return finished; }
    PauseActionResult getResult() const { return result; }

private:
    TTF_Font* titleFont;
    bool finished = false;
    PauseActionResult result = PauseActionResult::RESUME;
};