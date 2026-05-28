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
    VolumeScreen(float currentSfx, float currentMusic);

    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    VolumeResult getResult() const { return result; }

private:
    Slider* sfxSlider   = nullptr;
    Slider* musicSlider = nullptr;

    bool finished = false;
    VolumeResult result{};
    int selectedSlider = 0;  // 0 = sfx, 1 = music
};