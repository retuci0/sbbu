#include "TitleScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"

#include <SDL2/SDL_render.h>


TitleScreen::TitleScreen(SDL_Renderer* r, SDL_Texture* bg, TTF_Font* f)
    : renderer(r), bg(bg), font(f) {
    addWidget<Button>(SW/2-200, SH/2, 400, 80, "local play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::LOCAL; finished=true; });
    addWidget<Button>(SW/2-200, SH/2+120, 400, 80, "online play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::ONLINE; finished=true; });
}

void TitleScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) finished = true;
}

void TitleScreen::render(SDL_Renderer* r) {
    SDL_Rect bgRect = { 0, 0, SW, SH };
    SDL_RenderCopy(renderer, bg, nullptr, &bgRect);
    drawWidgets(r, font);
}
