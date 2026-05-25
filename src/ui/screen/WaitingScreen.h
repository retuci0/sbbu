#pragma once

#include "../Screen.h"


class WaitingScreen : public Screen {
public:
    WaitingScreen(SDL_Renderer* r, TTF_Font* titleFont, TTF_Font* font);
    void render(SDL_Renderer* r, TTF_Font* f) override;
    void update() override;

private:
    SDL_Renderer* renderer;
    TTF_Font *titleFont, *font;
    Uint32 startTime;
    int dotCount = 0;
};