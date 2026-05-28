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
};

class SettingsScreen : public Screen {
public:
    SettingsScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, int maxFps, bool vsync, bool fullscreen);

    void handle(const SDL_Event& event) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    OptionsResult getResult() const { return result; }

private:
    TTF_Font* font;
    TTF_Font* titleFont;

    Slider* fpsCapSlider     = nullptr;
    Button* fullscreenButton = nullptr;
    Button* vsyncButton      = nullptr;
    Button* okButton         = nullptr;
    bool finished = false;

    OptionsResult result{};
    int selectedIndex = 0;
};