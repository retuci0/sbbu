#include "pause.h"

#include "../misc/common.h"


PauseAction drawPauseScreen(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font,
                             const SDL_Event& event, int sw, int sh) {
    // semi-transparent dark overlay
    fillRect(renderer, 0, 0, sw, sh, 0, 0, 0, 150);

    // title bar
    fillRect(renderer, 200, 150, 1500, 150, 0, 0, 0, 255);
    renderText(renderer, titleFont, "game paused.", 750, 190, WHITE);

    // buttons
    SDL_Rect resume  = renderButton(renderer, font, "resume",           300,  450, 600, 150, {255,255,255,255}, BLACK);
    SDL_Rect quit    = renderButton(renderer, font, "quit",            1000,  450, 600, 150, {255,255,255,255}, BLACK);
    SDL_Rect chChar  = renderButton(renderer, font, "change characters", 300, 700, 600, 150, {255,255,255,255}, BLACK);
    SDL_Rect chVol   = renderButton(renderer, font, "change volume",   1000,  700, 600, 150, {255,255,255,255}, BLACK);

    // hit-test mouse clicks
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mouseX = event.button.x, mouseY = event.button.y;
        if (pointInRect(mouseX, mouseY, resume)) { return PauseAction::RESUME; }
        if (pointInRect(mouseX, mouseY, quit))   { return PauseAction::QUIT; }
        if (pointInRect(mouseX, mouseY, chChar)) { return PauseAction::CHANGE_CHARACTERS; }
        if (pointInRect(mouseX, mouseY, chVol))  { return PauseAction::CHANGE_VOLUME; }
    }
    return PauseAction::NONE;
}