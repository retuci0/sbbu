#pragma once

#include "Screen.h"

#include <SDL2/SDL.h>
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
    void handleEvent(const SDL_Event& e) override;
    void update() override;
    void render(SDL_Renderer* renderer) override;
    bool isFinished() const { return finished; }
    PauseActionResult getResult() const { return result; }

private:
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* font;
    int sw, sh;
    bool finished = false;
    PauseActionResult result = PauseActionResult::RESUME;
};