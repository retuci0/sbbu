#include "ui/screen/SettingsScreen.h"

#include "ui/widget/Button.h"
#include "ui/widget/Slider.h"
#include "misc/Color.h"

#include <SDL2/SDL_render.h>


static constexpr int SL_W = 700, SL_H = 20, SL_X = (SW - SL_W) / 2, SL_Y = 420;
static constexpr int BTN_W = 250, BTN_H = 70;
static constexpr Color BTN_YES_C = Color{ 50, 180, 50, 255 }, BTN_NO_C = Color{ 255, 50, 50, 255 };
static constexpr int ROW_1 = SW/2 - BTN_W - 20, ROW_2 = SW/2 + 20;
static constexpr int COL_1 = 520, COL_2 = 520 + BTN_H + 20;
static constexpr int OK_X = (SW - BTN_W) / 2, OK_Y = 750;

SettingsScreen::SettingsScreen(int maxFps, bool vsync, bool fullscreen, bool debug, bool particles) : Screen() {
    result = { maxFps, vsync, fullscreen, debug, particles };

    fpsCapSlider = addWidget<Slider>(SL_X, SL_Y, SL_W, SL_H, -1, 360, maxFps, "fps cap", false, false);

    fullscreenButton = addWidget<Button>(ROW_1, COL_1, BTN_W, BTN_H, "fullscreen", 
        result.fullscreen ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.fullscreen = !result.fullscreen; fullscreenButton->setColors(result.fullscreen ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    vsyncButton = addWidget<Button>(ROW_1, COL_2, BTN_W, BTN_H, "vsync", 
        result.vsync ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.vsync = !result.vsync; vsyncButton->setColors(result.vsync ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    debugButton = addWidget<Button>(ROW_2, COL_1, BTN_W, BTN_H, "debug",
        result.debug ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.debug = !result.debug; debugButton->setColors(result.debug ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    particlesButton = addWidget<Button>(ROW_2, COL_2, BTN_W, BTN_H, "particles",
        result.particles ? BTN_YES_C : BTN_NO_C, WHITE,
        [&]{ result.particles = !result.particles; particlesButton->setColors(result.particles ? BTN_YES_C : BTN_NO_C, WHITE); }
    );

    okButton = addWidget<Button>(OK_X, OK_Y, BTN_W, BTN_H, "ok.",
        Color{ 50, 180, 50, 255 }, WHITE,
        [&]{ result.fpsCap = fpsCapSlider->getValue(); finished = true; }
    );
}

void SettingsScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    static constexpr int WIDGET_COUNT = 6;
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
            } else if (active == debugButton) {
                result.debug = !result.debug;
                debugButton->setColors(result.debug ? BTN_YES_C : BTN_NO_C, WHITE);
            } else if (active == particlesButton) {
                result.particles = !result.particles;
                particlesButton->setColors(result.particles ? BTN_YES_C : BTN_NO_C, WHITE);  
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

    Widget* widgets[6] = { fpsCapSlider, fullscreenButton, vsyncButton, debugButton, particlesButton, okButton };
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