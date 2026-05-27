#pragma once

#include "../Screen.h"
#include "Options.h"

#include <SDL2/SDL_ttf.h>


enum class PauseActionResult {
    RESUME,
    QUIT,
    RESTART,
    CHANGE_VOLUME,
    CHANGE_CONTROLS
};

class PauseScreen : public Screen {
public:
    PauseScreen(SDL_Renderer* renderer, int sw, int sh, TTF_Font* titleFont, TTF_Font* font, Options options);

    void render(SDL_Renderer* renderer) override;
    void handle(const SDL_Event& event) override;

    bool isFinished() const { return finished; }
    PauseActionResult getResult() const { return result; }

private:
    TTF_Font* titleFont;
    TTF_Font* font;
    Options options;
    bool finished = false;
    PauseActionResult result = PauseActionResult::RESUME;
};
