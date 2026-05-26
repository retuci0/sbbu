#pragma once

#include "../Screen.h"
#include "../widget/Slider.h"

#include <SDL2/SDL_ttf.h>


struct VolumeResult {
    float sfx;
    float music;
};

class VolumeScreen : public Screen {
public:
    VolumeScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, float currentSfx, float currentMusic);

    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    VolumeResult getResult()  const { return result; }

private:
    TTF_Font* titleFont;
    TTF_Font* font;

    Slider* sfxSlider   = nullptr;
    Slider* musicSlider = nullptr;

    bool finished = false;
    VolumeResult result{};
};
