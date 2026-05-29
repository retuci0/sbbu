#include "VolumeScreen.h"

#include "ui/widget/Button.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_ttf.h>


static constexpr int SL_X = 600, SL_W = 700, SL_H = 20;
static constexpr int SL_SFX_Y = 420, SL_MUSIC_Y = 600;
static constexpr int OK_X = 860, OK_Y = 750, OK_W = 200, OK_H = 70;

VolumeScreen::VolumeScreen(float currentSfx, float currentMusic) : Screen() {
    sfxSlider   = addWidget<Slider>(SL_X, SL_SFX_Y,   SL_W, SL_H, 0.0f, 2.0f, currentSfx,   "SFX Volume");
    musicSlider = addWidget<Slider>(SL_X, SL_MUSIC_Y, SL_W, SL_H, 0.0f, 2.0f, currentMusic, "music Volume");

    addWidget<Button>(OK_X, OK_Y, OK_W, OK_H, "ok.",
        Color{50, 180, 50, 255}, WHITE,
        [&]{ finished = true; result = { sfxSlider->getValue(), musicSlider->getValue() }; }
    );
}

void VolumeScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    if (e.type != SDL_KEYDOWN) return;

    Slider* active = (selectedSlider == 0) ? sfxSlider : musicSlider;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_DOWN:
            selectedSlider = 1 - selectedSlider;
            break;
        case SDLK_LEFT:
            active->setValue(active->getValue() - 0.05f);
            break;
        case SDLK_RIGHT:
            active->setValue(active->getValue() + 0.05f);
            break;
        case SDLK_RETURN:
            finished = true;
            result   = { sfxSlider->getValue(), musicSlider->getValue() };
            break;
        case SDLK_ESCAPE:
            finished = true;
            result   = { sfxSlider->getValue(), musicSlider->getValue() };
            break;
        default: break;
    }
}

void VolumeScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, { 20, 20, 20, 200 });
    Renderer::renderText(renderer, titleFont, "volume settings", 750, 200, WHITE);
    drawWidgets(renderer, font);

    Slider* active = (selectedSlider == 0) ? sfxSlider : musicSlider;
    if (active) {
        Renderer::outlineRect(renderer,
            active->getX() - 4, active->getY() - 4,
            active->getW() + 8, active->getH() + 8,
            {255, 255, 255, 255}, 2);
    }
}

bool VolumeScreen::isTransparent() const {
    return true;
}