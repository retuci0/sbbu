#include "GameEndScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_ttf.h>


GameEndScreen::GameEndScreen(SDL_Renderer* /*renderer*/, TTF_Font* titleFont, TTF_Font* font,
                             const std::string& title, const std::string& details)
    : titleFont(titleFont), font(font), title(title), details(details)
{
    const int btnW = 360;
    const int btnH = 76;
    const int gap = 50;
    const int totalW = btnW * 2 + gap;
    const int x = (SW - totalW) / 2;
    const int y = 700;

    addWidget<Button>(x, y, btnW, btnH, "title screen", WHITE, BLACK, [&] {
        result = GameEndActionResult::TITLE;
        finished = true;
    });
    addWidget<Button>(x + btnW + gap, y, btnW, btnH, "quit", WHITE, BLACK, [&] {
        result = GameEndActionResult::QUIT;
        finished = true;
    });
}

void GameEndScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE) {
            result = GameEndActionResult::TITLE;
            finished = true;
        }
    }
}

void GameEndScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 180});

    int tw = 0, th = 0;
    TTF_SizeText(titleFont, title.c_str(), &tw, &th);
    Renderer::renderText(renderer, titleFont, title, (SW - tw) / 2, 260, WHITE);

    TTF_SizeText(font, details.c_str(), &tw, &th);
    Renderer::renderText(renderer, font, details, (SW - tw) / 2, 390, {220, 220, 220, 255});

    drawWidgets(renderer, font);
}
