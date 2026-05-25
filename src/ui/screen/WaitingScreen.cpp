#include "WaitingScreen.h"

#include "../../misc/Renderer.h"
#include "../../misc/Common.h"


WaitingScreen::WaitingScreen(SDL_Renderer* r, TTF_Font* tf, TTF_Font* f)
    : renderer(r), titleFont(tf), font(f), startTime(SDL_GetTicks()) {}

void WaitingScreen::update() {
    if ((SDL_GetTicks() - startTime) / 500 > dotCount) {
        dotCount = (dotCount + 1) % 4;
    }
}

void WaitingScreen::render(SDL_Renderer* r, TTF_Font* f) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20,20,40});
    std::string dots(dotCount, '.');
    std::string msg = "waiting for host to start the game" + dots;
    int tw, th;
    TTF_SizeText(titleFont, msg.c_str(), &tw, &th);
    Renderer::renderText(r, titleFont, msg, (SW-tw)/2, (SH-th)/2, WHITE);
}