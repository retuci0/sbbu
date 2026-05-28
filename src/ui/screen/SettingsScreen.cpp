#include "SettingsScreen.h"

#include "../../misc/Color.h"
#include "../widget/Button.h"
#include "../widget/Slider.h"

#include <SDL2/SDL_render.h>


static constexpr int SL_W = 700, SL_H = 20, SL_X = (SW - SL_W) / 2, SL_Y = 420;
static constexpr int BTN_W = 250, BTN_H = 70, BTN_Y = 520;
static constexpr Color BTN_YES_C = Color{ 50, 180, 50, 255 }, BTN_NO_C = Color{ 255, 50, 50, 255 };
static constexpr int FS_X = SW / 2 - 2 * BTN_W - 20, VS_X = SW / 2 + BTN_W + 20;
static constexpr int OK_X = (SW - BTN_W) / 2, OK_Y = 750;

SettingsScreen::SettingsScreen(int maxFps, bool vsync, bool fullscreen) : Screen() {
    result = { maxFps, vsync, fullscreen };

    fpsCapSlider = addWidget<Slider>(SL_X, SL_Y, SL_W, SL_H, -1, 360, maxFps, "fps cap", false, false);

    fullscreenButton = addWidget<Button>(FS_X, BTN_Y, BTN_W, BTN_H, "fullscreen", 
        result.fullscreen ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.fullscreen = !result.fullscreen; fullscreenButton->setColors(result.fullscreen ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    vsyncButton = addWidget<Button>(VS_X, BTN_Y, BTN_W, BTN_H, "vsync", 
        result.vsync ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.vsync = !result.vsync; vsyncButton->setColors(result.vsync ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    okButton = addWidget<Button>(OK_X, OK_Y, BTN_W, BTN_H, "ok.",
        Color{ 50, 180, 50, 255 }, WHITE,
        [&]{ result.fpsCap = fpsCapSlider->getValue(); finished = true; }
    );
}

void SettingsScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    static constexpr int WIDGET_COUNT = 4;
    Widget* widgets[WIDGET_COUNT] = { fpsCapSlider, fullscreenButton, vsyncButton, okButton };
    Widget* active = widgets[selectedIndex];

    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            selectedIndex = (selectedIndex - 1 + WIDGET_COUNT) % WIDGET_COUNT;
            break;
        case SDLK_DOWN:
            selectedIndex = (selectedIndex + 1) % WIDGET_COUNT;
            break;
        case SDLK_LEFT:
            if (auto* sl = dynamic_cast<Slider*>(active)) {
                sl->setValue(sl->getValue() - 10);
            }
            break;
        case SDLK_RIGHT:
            if (auto* sl = dynamic_cast<Slider*>(active)) {
                sl->setValue(sl->getValue() + 10);
            }
            break;
        case SDLK_RETURN:
            if (active == fullscreenButton) {
                result.fullscreen = !result.fullscreen;
                fullscreenButton->setColors(result.fullscreen ? BTN_YES_C : BTN_NO_C, WHITE);
            } else if (active == vsyncButton) {
                result.vsync = !result.vsync;
                vsyncButton->setColors(result.vsync ? BTN_YES_C : BTN_NO_C, WHITE);
            } else if (active == okButton) {
                result.fpsCap = fpsCapSlider->getValue();
                finished = true;
            }
            break;
        case SDLK_ESCAPE:
            result.fpsCap = fpsCapSlider->getValue();
            finished = true;
            break;
        default: 
            break;
    }
}

void SettingsScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, { 20, 20, 20, 200 });
    Renderer::renderText(renderer, titleFont, "video settings", 750, 200, WHITE);
    drawWidgets(renderer, font);

    Widget* widgets[4] = { fpsCapSlider, fullscreenButton, vsyncButton, okButton };
    Widget* active = widgets[selectedIndex];
    if (active) {
        Renderer::outlineRect(renderer,
            active->getX() - 4, active->getY() - 4,
            active->getW() + 8, active->getH() + 8,
            { 255, 255, 255, 255 }, 2);
    }
}

bool SettingsScreen::isTransparent() const {
    return true;
}