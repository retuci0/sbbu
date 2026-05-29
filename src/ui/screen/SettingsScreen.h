#pragma once

#include "ui/Screen.h"

#include "ui/widget/Button.h"
#include "ui/widget/Slider.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


struct OptionsResult {
    int fpsCap;
    bool vsync;
    bool fullscreen;
    bool debug;
};

class SettingsScreen : public Screen {
public:
    SettingsScreen(int maxFps, bool vsync, bool fullscreen, bool debug);

    void handle(const SDL_Event& event) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    OptionsResult getResult() const { return result; }

private:
    Slider* fpsCapSlider     = nullptr;
    Button* fullscreenButton = nullptr;
    Button* vsyncButton      = nullptr;
    Button* debugButton      = nullptr;
    Button* okButton         = nullptr;
    bool finished = false;

    OptionsResult result{};
    int selectedIndex = 0;
};