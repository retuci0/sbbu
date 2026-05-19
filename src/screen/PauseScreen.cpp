#include "PauseScreen.h"

#include "../misc/Common.h"
#include "../misc/Renderer.h"


PauseScreen::PauseScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font, int sw, int sh)
    : renderer(renderer), titleFont(titleFont), font(font), sw(sw), sh(sh) {}

void PauseScreen::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        SDL_Rect resume = {300, 450, 600, 150};
        SDL_Rect quit = {1000, 450, 600, 150};
        SDL_Rect restart = {300, 700, 600, 150};
        SDL_Rect chVol = {1000, 700, 600, 150};
        if (pointInRect(mx, my, resume)) { result = PauseActionResult::RESUME; finished = true; }
        else if (pointInRect(mx, my, quit)) { result = PauseActionResult::QUIT; finished = true; }
        else if (pointInRect(mx, my, restart)) { result = PauseActionResult::RESTART; finished = true; }
        else if (pointInRect(mx, my, chVol)) { result = PauseActionResult::CHANGE_VOLUME; finished = true; }
    }
}

void PauseScreen::update() {}

void PauseScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, sw, sh, { 0, 0, 0, 150 });
    Renderer::fillRect(renderer, 200, 150, 1500, 150,BLACK);
    Renderer::renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);
    Renderer::renderButton(renderer, font, "resume", 300, 450, 600, 150, {255,255,255,255}, BLACK);
    Renderer::renderButton(renderer, font, "quit", 1000, 450, 600, 150, {255,255,255,255}, BLACK);
    Renderer::renderButton(renderer, font, "restart", 300, 700, 600, 150, {255,255,255,255}, BLACK);
    Renderer::renderButton(renderer, font, "change volume", 1000, 700, 600, 150, {255,255,255,255}, BLACK);
}