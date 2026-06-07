#pragma once

#include "ui/Screen.h"

#include "ui/widget/ButtonWidget.h"
#include "ui/widget/SliderWidget.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


struct OptionsResult {
    int fpsCap;
    bool vsync;
    bool fullscreen;
    bool debug;
    bool particles;
};

class SettingsScreen : public Screen {
public:
    SettingsScreen(int maxFps, bool vsync, bool fullscreen, bool debug, bool particles);

    void handle(const SDL_Event& event) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    OptionsResult getResult() const { return result; }

private:
    SliderWidget* fpsCapSlider     = nullptr;
    ButtonWidget* fullscreenButton = nullptr;
    ButtonWidget* vsyncButton      = nullptr;
    ButtonWidget* debugButton      = nullptr;
    ButtonWidget* particlesButton  = nullptr;
    ButtonWidget* okButton         = nullptr;
    bool finished = false;

    OptionsResult result{};
    int selectedIndex = 0;
};