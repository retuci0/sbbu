#include "PauseScreen.h"

#include "../../misc/Common.h"
#include "../../misc/Renderer.h"


PauseScreen::PauseScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, int sw, int sh)
    : titleFont(titleFont), font(font)
{
    Color bg = {255, 255, 255, 255};
    buttons.emplace_back(300,  450, 600, 150, "resume",        bg, BLACK, [&]{ result = PauseActionResult::RESUME;        finished = true; });
    buttons.emplace_back(1000, 450, 600, 150, "quit",          bg, BLACK, [&]{ result = PauseActionResult::QUIT;          finished = true; });
    buttons.emplace_back(300,  700, 600, 150, "restart",       bg, BLACK, [&]{ result = PauseActionResult::RESTART;       finished = true; });
    buttons.emplace_back(1000, 700, 600, 150, "change volume", bg, BLACK, [&]{ result = PauseActionResult::CHANGE_VOLUME; finished = true; });
}

void PauseScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 150});
    Renderer::fillRect(renderer, 200, 150, 1500, 150, BLACK);
    Renderer::renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);
    for (auto& b : buttons) b.draw(renderer, font);
}