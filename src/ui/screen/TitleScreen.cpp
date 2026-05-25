#include "TitleScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"


TitleScreen::TitleScreen(SDL_Renderer* r, TTF_Font* tf, TTF_Font* f)
    : renderer(r), titleFont(tf), font(f) {
    addWidget<Button>(SW/2-200, SH/2-80, 400, 80, "local play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::LOCAL; finished=true; });
    addWidget<Button>(SW/2-200, SH/2+40, 400, 80, "online play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::ONLINE; finished=true; });
}

void TitleScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) finished = true;
}

void TitleScreen::render(SDL_Renderer* r, TTF_Font* f) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20,20,40});
    Renderer::renderText(r, titleFont, "Super Bert Bros Ultimate", SW/2-250, 200, WHITE);
    drawWidgets(r, f);
}