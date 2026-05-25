#include "PauseScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_ttf.h>


PauseScreen::PauseScreen(SDL_Renderer* /*renderer*/, int /*sw*/, int /*sh*/, TTF_Font* titleFont)
    : titleFont(titleFont)
{
    const Color bg = {255, 255, 255, 255};

    addWidget<Button>(300,  450, 600, 150, "resume",        bg, BLACK, [&]{ result = PauseActionResult::RESUME;        finished = true; });
    addWidget<Button>(1000, 450, 600, 150, "quit",          bg, BLACK, [&]{ result = PauseActionResult::QUIT;          finished = true; });
    addWidget<Button>(300,  700, 600, 150, "restart",       bg, BLACK, [&]{ result = PauseActionResult::RESTART;       finished = true; });
    addWidget<Button>(1000, 700, 600, 150, "change volume", bg, BLACK, [&]{ result = PauseActionResult::CHANGE_VOLUME; finished = true; });
}

void PauseScreen::render(SDL_Renderer* renderer, TTF_Font* font) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 150});
    Renderer::fillRect(renderer, 200, 150, 1500, 150, BLACK);
    Renderer::renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);
    drawWidgets(renderer, font);
}