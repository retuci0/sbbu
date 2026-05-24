#pragma once

#include "../Screen.h"

#include <SDL2/SDL_ttf.h>

#include <string>


struct VolumeResult { 
    float sfx; 
    float music; 
};

class VolumeScreen : public Screen {
public:
    VolumeScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, float currentSfx, float currentMusic);
    void handleEvent(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;
    bool isFinished() const { return finished; }
    VolumeResult getResult() const { return result; }

private:
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* font;
    float sfx, music;
    int dragging = 0;
    bool finished = false;
    VolumeResult result;

    void drawSlider(int slY, float val, const std::string& label);
    int valToX(float v) const;
    float xToVal(int x) const;
};