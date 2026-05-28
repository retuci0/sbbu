#include "TitleScreen.h"

#include "../widget/Button.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_render.h>


TitleScreen::TitleScreen() : Screen() {
    bg = Resources::get().getTexture("title_bg");
    addWidget<Button>(SW/2-200, SH/2, 400, 80, "local play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::LOCAL; finished=true; });
    addWidget<Button>(SW/2-200, SH/2+120, 400, 80, "online play",
                      Color{60,60,60}, WHITE, [&]{ result = MultiplayerModeResult::ONLINE; finished=true; });
}

void TitleScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                finished = true;
                break;
            case SDLK_UP:
                selectedIndex = (selectedIndex + 1) % 2;  // wrap around
                break;
            case SDLK_DOWN:
                selectedIndex = (selectedIndex + 1) % 2;
                break;
            case SDLK_RETURN:
                if (selectedIndex == 0)
                    result = MultiplayerModeResult::LOCAL;
                else
                    result = MultiplayerModeResult::ONLINE;
                finished = true;
                break;
        }
    }
}

void TitleScreen::render(SDL_Renderer* r) {
    SDL_Rect bgRect = { 0, 0, SW, SH };
    SDL_RenderCopy(r, bg, nullptr, &bgRect);
    for (size_t i = 0; i < widgets.size(); ++i) {
        auto* btn = dynamic_cast<Button*>(widgets[i].get());
        if (btn) {
            btn->draw(r, font);
            if (i == selectedIndex) {
                Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
            }
        }
    }
}