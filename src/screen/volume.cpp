#include "volume.h"

#include "../misc/common.h"

#include <algorithm>
#include <string>
#include <cmath>


// slider geometry
static constexpr int SL_X = 600, SL_W = 700, SL_H = 20;
static constexpr int SL_SFX_Y   = 420;
static constexpr int SL_MUSIC_Y = 600;
static constexpr int HANDLE_R   = 18;
static constexpr int OK_X = 860, OK_Y = 750, OK_W = 200, OK_H = 70;

// map value [0, 2] to x position on slider
static int valToX(float v) {
    return SL_X + static_cast<int>(v / 2.0f * SL_W);
}
// map x position to value [0, 2]
static float xToVal(int x) {
    return std::clamp((x - SL_X) / static_cast<float>(SL_W) * 2.0f, 0.0f, 2.0f);
}

static void drawSlider(SDL_Renderer* r, TTF_Font* font, const std::string& label, float val, int slY) {
    // track
    fillRect(r, SL_X, slY, SL_W, SL_H, 100, 100, 100, 255);
    // fill
    int fillW = valToX(val) - SL_X;
    fillRect(r, SL_X, slY, fillW, SL_H, 100, 180, 100, 255);
    // handle
    int hx = valToX(val);
    fillRect(r, hx - HANDLE_R, slY - HANDLE_R + SL_H/2,
                HANDLE_R * 2, HANDLE_R * 2, 200, 200, 200, 255);
    // label + value
    renderText(r, font, label + ": " + std::to_string(static_cast<int>(std::round(val * 100))) + "%",
               SL_X, slY - 50, WHITE);
}

VolumeResult runVolumeScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font,
                              float currentSfx, float currentMusic) {
    float sfx = currentSfx;
    float music = currentMusic;
    int dragging = 0; // 0 = none, 1 = sfx, 2 = music

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }
            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                int mouseX = ev.button.x, mouseY = ev.button.y;
                SDL_Rect okBtn = {OK_X, OK_Y, OK_W, OK_H};
                if (pointInRect(mouseX, mouseY, okBtn)) { running = false; break; }
                if (std::abs(mouseY - (SL_SFX_Y   + SL_H / 2)) < HANDLE_R * 2) { dragging = 1; }
                if (std::abs(mouseY - (SL_MUSIC_Y + SL_H / 2)) < HANDLE_R * 2) { dragging = 2; }
            }
            if (ev.type == SDL_MOUSEBUTTONUP) { dragging = 0; }
            if (ev.type == SDL_MOUSEMOTION && dragging) {
                float val = xToVal(ev.motion.x);
                if (dragging == 1) { sfx   = val; }
                else               { music = val; }
            }
        }

        fillRect(renderer, 0, 0, 1920, 1080, 20, 20, 20, 255);
        renderText(renderer, titleFont, "volume Settings", 750, 200, WHITE);
        drawSlider(renderer, font, "SFX Volume",   sfx,   SL_SFX_Y);
        drawSlider(renderer, font, "music Volume", music, SL_MUSIC_Y);
        renderButton(renderer, font, "ok.", OK_X, OK_Y, OK_W, OK_H, {50,180,50,255}, WHITE);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    return {sfx, music};
}