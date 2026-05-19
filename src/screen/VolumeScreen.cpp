#include "VolumeScreen.h"

#include "../misc/Common.h"
#include "../misc/Renderer.h"

#include <algorithm>
#include <cmath>
#include <string>

static constexpr int SL_X = 600, SL_W = 700, SL_H = 20;
static constexpr int SL_SFX_Y = 420, SL_MUSIC_Y = 600;
static constexpr int HANDLE_R = 18;
static constexpr int OK_X = 860, OK_Y = 750, OK_W = 200, OK_H = 70;


VolumeScreen::VolumeScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, float currentSfx, float currentMusic)
    : renderer(renderer), titleFont(titleFont), font(font), sfx(currentSfx), music(currentMusic) {}

int VolumeScreen::valToX(float v) const {
    return SL_X + static_cast<int>(v / 2.0f * SL_W);
}
float VolumeScreen::xToVal(int x) const {
    return std::clamp((x - SL_X) / static_cast<float>(SL_W) * 2.0f, 0.0f, 2.0f);
}

void VolumeScreen::drawSlider(int slY, float val, const std::string& label) {
    Renderer::fillRect(renderer, SL_X, slY, SL_W, SL_H, { 100, 100, 100, 255 });
    int fillW = valToX(val) - SL_X;
    Renderer::fillRect(renderer, SL_X, slY, fillW, SL_H, { 100, 180, 100, 255 });
    int hx = valToX(val);
    Renderer::fillRect(renderer, hx - HANDLE_R, slY - HANDLE_R + SL_H/2, HANDLE_R*2, HANDLE_R*2, { 200, 200, 200, 255 });
    Renderer::renderText(renderer, font, label + ": " + std::to_string(static_cast<int>(std::round(val * 100))) + "%",
                         SL_X, slY - 50, WHITE);
}

void VolumeScreen::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_QUIT) { finished = true; return; }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        SDL_Rect okBtn = {OK_X, OK_Y, OK_W, OK_H};
        if (pointInRect(mx, my, okBtn)) { finished = true; result = {sfx, music}; return; }
        if (std::abs(my - (SL_SFX_Y + SL_H/2)) < HANDLE_R*2) dragging = 1;
        if (std::abs(my - (SL_MUSIC_Y + SL_H/2)) < HANDLE_R*2) dragging = 2;
    }
    if (e.type == SDL_MOUSEBUTTONUP) dragging = 0;
    if (e.type == SDL_MOUSEMOTION && dragging) {
        float val = xToVal(e.motion.x);
        if (dragging == 1) sfx = val;
        else music = val;
    }
}

void VolumeScreen::update() {
    // nun to do
}

void VolumeScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, 1920, 1080, { 20, 20, 20, 255 });
    Renderer::renderText(renderer, titleFont, "volume settings", 750, 200, WHITE);
    drawSlider(SL_SFX_Y, sfx, "SFX Volume");
    drawSlider(SL_MUSIC_Y, music, "Music Volume");
    Renderer::renderButton(renderer, font, "ok", OK_X, OK_Y, OK_W, OK_H, {50,180,50,255}, WHITE);
}