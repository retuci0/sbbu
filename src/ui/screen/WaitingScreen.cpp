#include "ui/screen/WaitingScreen.h"

#include "ui/widget/ButtonWidget.h"
#include "misc/Renderer.h"
#include "misc/Common.h"


WaitingScreen::WaitingScreen()
    : startTime(SDL_GetTicks()), Screen()  
{
    addWidget<ButtonWidget>(4, 4, 64, 64, "<", GREEN, WHITE, [&]{ goBack(); });
}

void WaitingScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        goBack();
    }
}

void WaitingScreen::update() {
    Uint32 elapsed = SDL_GetTicks() - startTime;
    dotCount = (elapsed / 500) % 4;
}

void WaitingScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20,20,40});
    std::string dots(dotCount, '.');
    std::string msg = "waiting for host to start the game" + dots;
    int tw, th;
    TTF_SizeText(titleFont, msg.c_str(), &tw, &th);
    Renderer::renderText(r, titleFont, msg, (SW-tw)/2, (SH-th)/2, WHITE);
    Screen::drawWidgets(r, font);
}
