#pragma once

#include "ui/Screen.h"


class WaitingScreen : public Screen {
public:
    WaitingScreen();
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r) override;
    void update() override;

private:
    Uint32 startTime;
    int dotCount = 0;
};
