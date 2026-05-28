#include "GameEndScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_ttf.h>


GameEndScreen::GameEndScreen(SDL_Renderer* /*renderer*/, TTF_Font* titleFont, TTF_Font* font,
                             const std::string& title, const std::string& details)
    : titleFont(titleFont), font(font), title(title), details(details)
{
    const int BTN_W = 360, BTN_H = 76;
    const int GAP = 50;
    const int w = BTN_W * 2 + GAP;
    const int x = (SW - w) / 2;
    const int y = 700;

    addWidget<Button>(x, y, BTN_W, BTN_H, "title screen", WHITE, BLACK, [&] {
        result = GameEndActionResult::TITLE;
        finished = true;
    });
    addWidget<Button>(x + BTN_W + GAP, y, BTN_W, BTN_H, "quit", WHITE, BLACK, [&] {
        result = GameEndActionResult::QUIT;
        finished = true;
    });
}

void GameEndScreen::handle(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                selectedIndex = 1 - selectedIndex;
                return;
            case SDLK_RETURN:
                result   = (selectedIndex == 0) ? GameEndActionResult::TITLE
                                                : GameEndActionResult::QUIT;
                finished = true;
                return;
            case SDLK_ESCAPE:
                result   = GameEndActionResult::TITLE;
                finished = true;
                return;
            default: break;
        }
    }
    Screen::handle(e);
}

void GameEndScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 180});

    int tw = 0, th = 0;
    TTF_SizeText(titleFont, title.c_str(), &tw, &th);
    Renderer::renderText(renderer, titleFont, title, (SW - tw) / 2, 260, WHITE);

    TTF_SizeText(font, details.c_str(), &tw, &th);
    Renderer::renderText(renderer, font, details, (SW - tw) / 2, 390, {220, 220, 220, 255});

    drawWidgets(renderer, font);

    if (selectedIndex >= 0 && selectedIndex < (int)widgets.size()) {
        auto* btn = dynamic_cast<Button*>(widgets[selectedIndex].get());
        if (btn) {
            Renderer::outlineRect(renderer, btn->getX(), btn->getY(),
                    btn->getW(), btn->getH(), {255,255,255,255}, 3);
        }
    }
}