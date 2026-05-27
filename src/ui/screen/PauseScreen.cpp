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
    : titleFont(titleFont), font(font), options(options)
{
    const Color bg = {255, 255, 255, 255};

    const int btnW = 400;
    const int btnH = 80;
    const int gap  = 40;
    const int colGap = 60;

    int totalWidth  = 2 * btnW + colGap;
    int startX      = (sw - totalWidth) / 2;
    int startY      = sh / 3 + 100;

    // row 0
    addWidget<Button>(startX,                     startY, btnW, btnH, "resume",        bg, BLACK, [&]{ result = PauseActionResult::RESUME;          finished = true; });
    addWidget<Button>(startX + btnW + colGap,     startY, btnW, btnH, "quit",          bg, BLACK, [&]{ result = PauseActionResult::QUIT;            finished = true; });

    // row 1
    addWidget<Button>(startX,                     startY + btnH + gap, btnW, btnH, "restart",       bg, BLACK, [&]{ result = PauseActionResult::RESTART;         finished = true; });
    addWidget<Button>(startX + btnW + colGap,     startY + btnH + gap, btnW, btnH, "change volume", bg, BLACK, [&]{ result = PauseActionResult::CHANGE_VOLUME;   finished = true; });

    // row 2
    addWidget<Button>(startX,                     startY + 2 * (btnH + gap), btnW, btnH, "controls",      bg, BLACK, [&]{ result = PauseActionResult::CHANGE_CONTROLS; finished = true; });
    addWidget<Button>(startX + btnW + colGap,     startY + 2 * (btnH + gap), btnW, btnH, "assets folder", bg, BLACK,  openAssetsFolder);
}

void PauseScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 150});
    Renderer::fillRect(renderer, 200, 150, 1500, 150, BLACK);
    Renderer::renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);
    drawWidgets(renderer, font);
}

void PauseScreen::handle(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == options.keyPause) {
        finished = true;
    }
    Screen::handle(event);
}