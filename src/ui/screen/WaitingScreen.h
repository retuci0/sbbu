#pragma once

#include "ui/Screen.h"


class WaitingScreen : public Screen {
public:
    WaitingScreen();
    void render(SDL_Renderer* r) override;
    void update() override;
    bool shouldGoBack() const { return goBack; }

private:
    Uint32 startTime;
    int dotCount = 0;
    bool goBack = false;
};
