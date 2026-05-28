#include "PauseScreen.h"
#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"
#include "../../Options.h"
#include "ui/Screen.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_ttf.h>
#include <filesystem>

void openAssetsFolder() {
    openFolder(std::filesystem::current_path().string() + "/assets/");
}

PauseScreen::PauseScreen(SDL_Renderer* /*renderer*/, int sw, int sh, TTF_Font* titleFont, TTF_Font* font, Options options)
    : titleFont(titleFont), font(font), options(options), selectedIndex(0)
{
    const Color bg = {255,255,255,255};
    const int btnW=400, btnH=80, gap=40, colGap=60;
    int totalWidth = 2*btnW + colGap;
    int startX = (sw - totalWidth)/2;
    int startY = sh/3 + 100;

    addWidget<Button>(startX,                     startY, btnW, btnH, "resume",        bg, BLACK, [&]{ result = PauseActionResult::RESUME;          finished = true; });
    addWidget<Button>(startX+btnW+colGap,         startY, btnW, btnH, "quit",          bg, BLACK, [&]{ result = PauseActionResult::QUIT;            finished = true; });
    addWidget<Button>(startX,                     startY+btnH+gap, btnW, btnH, "restart",       bg, BLACK, [&]{ result = PauseActionResult::RESTART;         finished = true; });
    addWidget<Button>(startX+btnW+colGap,         startY+btnH+gap, btnW, btnH, "change volume", bg, BLACK, [&]{ result = PauseActionResult::CHANGE_VOLUME;   finished = true; });
    addWidget<Button>(startX,                     startY+2*(btnH+gap), btnW, btnH, "controls",      bg, BLACK, [&]{ result = PauseActionResult::CHANGE_CONTROLS; finished = true; });
    addWidget<Button>(startX+btnW+colGap,         startY+2*(btnH+gap), btnW, btnH, "assets folder", bg, BLACK,  openAssetsFolder);
}

void PauseScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0,0,0,150});
    Renderer::fillRect(renderer, 200, 150, 1500, 150, GRAY);
    Renderer::renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);
    for (size_t i = 0; i < widgets.size(); ++i) {
        auto* btn = dynamic_cast<Button*>(widgets[i].get());
        if (btn) {
            btn->draw(renderer, font);
            if (i == (size_t) selectedIndex) {
                Renderer::outlineRect(renderer, btn->getX(), btn->getY(), btn->getW(), btn->getH(), BLACK, 3);
            }
        }
    }
}

void PauseScreen::handle(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                finished = true;
                return;
            case SDLK_UP:
                if (selectedIndex >= 2) selectedIndex -= 2;
                break;
            case SDLK_DOWN:
                if (selectedIndex + 2 < (int)widgets.size()) selectedIndex += 2;
                break;
            case SDLK_LEFT:
                if (selectedIndex % 2 == 1) selectedIndex--;
                break;
            case SDLK_RIGHT:
                if (selectedIndex % 2 == 0 && selectedIndex + 1 < (int)widgets.size()) selectedIndex++;
                break;
            case SDLK_RETURN: {
                auto* btn = dynamic_cast<Button*>(widgets[selectedIndex].get());
                if (btn) btn->activate();
                return;
            }
        }
    }
    Screen::handle(event);
}