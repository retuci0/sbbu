#pragma once

#include "../Screen.h"


class WaitingScreen : public Screen {
public:
    WaitingScreen();
    void render(SDL_Renderer* r) override;
    void update() override;

private:
    Uint32 startTime;
    int dotCount = 0;
};
