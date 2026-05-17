#include "character_selection.h"

#include "../misc/common.h"

#include <SDL2/SDL.h>

#include <SDL2/SDL_rect.h>
#include <array>


static constexpr int SW = 1920, SH = 1080;

static constexpr int BOX_W = 360, BOX_H = 100;
static constexpr int COL1_X = 100,  COL2_X = 1050;
static constexpr int ROW_START_Y = 200, ROW_STEP = 120;

static constexpr int NAME_BOX_X1 = 100,  NAME_BOX_X2 = 1050;
static constexpr int NAME_BOX_Y  = 720,  NAME_BOX_W  = 360, NAME_BOX_H = 60;

static constexpr int START_BTN_X = 760, START_BTN_Y = 870;
static constexpr int START_BTN_W = 400, START_BTN_H = 80;

SelectionResult runCharacterSelection(
    SDL_Renderer*    renderer,
    TTF_Font*        titleFont,
    TTF_Font*        font,
    const Character* chars[4],
    const std::string& defaultName1, const Character* defaultChar1,
    const std::string& defaultName2, const Character* defaultChar2)
{
    auto findIdx = [&](const Character* ch) -> int {
        for (int i = 0; i < 4; ++i) if (chars[i] == ch) return i;
        return 0;
    };
    int sel1 = findIdx(defaultChar1);
    int sel2 = findIdx(defaultChar2);

    std::string name1 = defaultName1;
    std::string name2 = defaultName2;

    int activeField = 0;

    SDL_StartTextInput();
    bool running = true;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }

            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_RETURN) {
                    running = false;
                } else if (ev.key.keysym.sym == SDLK_BACKSPACE) {
                    if (activeField == 1 && !name1.empty()) name1.pop_back();
                    if (activeField == 2 && !name2.empty()) name2.pop_back();
                }
            }
            if (ev.type == SDL_TEXTINPUT) {
                if (activeField == 1 && name1.size() < 16) name1 += ev.text.text;
                if (activeField == 2 && name2.size() < 16) name2 += ev.text.text;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                int mx = ev.button.x, my = ev.button.y;

                for (int i = 0; i < 4; ++i) {
                    SDL_Rect box = {COL1_X, ROW_START_Y + i * ROW_STEP, BOX_W, BOX_H};
                    if (pointInRect(mx, my, box)) sel1 = i;
                }
                for (int i = 0; i < 4; ++i) {
                    SDL_Rect box = {COL2_X, ROW_START_Y + i * ROW_STEP, BOX_W, BOX_H};
                    if (pointInRect(mx, my, box)) sel2 = i;
                }
                SDL_Rect nf1 = {NAME_BOX_X1, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H};
                SDL_Rect nf2 = {NAME_BOX_X2, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H};
                activeField = pointInRect(mx, my, nf1) 
                        ? 1
                        : pointInRect(mx, my, nf2) 
                            ? 2 
                            : 0;

                SDL_Rect startBtn = {START_BTN_X, START_BTN_Y, START_BTN_W, START_BTN_H};
                if (pointInRect(mx, my, startBtn)) running = false;
            }
        }

        fillRect(renderer, 0, 0, SW, SH, 30, 30, 30, 255);

        renderText(renderer, titleFont, "character selection", 650, 60, WHITE);
        renderText(renderer, font, "player 1", COL1_X, 150, WHITE);
        renderText(renderer, font, "player 2", COL2_X, 150, WHITE);

        const std::array<std::string, 4> charNames = {
            "Bert (balanced)", "Berrota (fast)", "Lorc (tank)", "Jordi (glass cannon)"
        };

        for (int i = 0; i < 4; ++i) {
            // P1
            SDL_Color bg1 = (sel1 == i) ? SDL_Color{80,150,80,255} : SDL_Color{60,60,60,255};
            renderButton(renderer, font, charNames[i],
                         COL1_X, ROW_START_Y + i * ROW_STEP, BOX_W, BOX_H, bg1, WHITE);
            // P2
            SDL_Color bg2 = (sel2 == i) ? SDL_Color{80,150,80,255} : SDL_Color{60,60,60,255};
            renderButton(renderer, font, charNames[i],
                         COL2_X, ROW_START_Y + i * ROW_STEP, BOX_W, BOX_H, bg2, WHITE);
        }

        // character icons
        if (chars[sel1] && chars[sel1]->icon) {
            SDL_Rect iconRect1 = { 500, ROW_START_Y, 125, 57 };
            SDL_RenderCopy(renderer, chars[sel1]->icon, nullptr, &iconRect1);
        }
        if (chars[sel2] && chars[sel2]->icon) {
            SDL_Rect iconRect2 = { 1450, ROW_START_Y, 125, 57 };
            SDL_RenderCopy(renderer, chars[sel2]->icon, nullptr, &iconRect2);
        }

        renderText(renderer, font, "Enter name:", COL1_X, NAME_BOX_Y - 40, WHITE);
        renderText(renderer, font, "Enter name:", COL2_X, NAME_BOX_Y - 40, WHITE);

        SDL_Color nf1bg = (activeField == 1) ? SDL_Color{100,100,180,255} : SDL_Color{60,60,60,255};
        SDL_Color nf2bg = (activeField == 2) ? SDL_Color{100,100,180,255} : SDL_Color{60,60,60,255};
        renderButton(renderer, font, name1, NAME_BOX_X1, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, nf1bg, WHITE);
        renderButton(renderer, font, name2, NAME_BOX_X2, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, nf2bg, WHITE);

        renderButton(renderer, titleFont, "start game",
                     START_BTN_X, START_BTN_Y, START_BTN_W, START_BTN_H,
                     {50,180,50,255}, WHITE);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (name1.empty()) name1 = "player 1";
    if (name2.empty()) name2 = "player 2";

    return { chars[sel1], chars[sel2], name1, name2 };
}