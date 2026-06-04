#include "ui/screen/ControlsScreen.h"

#include "ui/widget/Button.h"
#include "ui/widget/KeybindWidget.h"

#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_ttf.h>


static constexpr int COL1_X = 250, COL2_X = 1050;
static constexpr int ROW_Y = 160, ROW_STEP = 70;
static constexpr int ROW_W = 600, ROW_H = 55;

ControlsScreen::ControlsScreen(Options& o, InputHandler& input)
    : selectedRow(0), selectedColumn(0), Screen() 
{
    struct Row { const char* name; SDL_KeyCode& key; SDL_GameControllerButton btn; };
    Row p1[] = {
        {"left",    o.keyP1Left,    SDL_CONTROLLER_BUTTON_INVALID},
        {"right",   o.keyP1Right,   SDL_CONTROLLER_BUTTON_INVALID},
        {"down",    o.keyP1Down,    SDL_CONTROLLER_BUTTON_INVALID},
        {"jump",    o.keyP1Jump,    SDL_CONTROLLER_BUTTON_A},
        {"shoot",   o.keyP1Shoot,   SDL_CONTROLLER_BUTTON_B},
        {"melee",   o.keyP1Melee,   SDL_CONTROLLER_BUTTON_X},
        {"special", o.keyP1Special, SDL_CONTROLLER_BUTTON_Y},
        {"shield",  o.keyP1Shield,  SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
    };
    Row p2[] = {
        {"left",    o.keyP2Left,    SDL_CONTROLLER_BUTTON_INVALID},
        {"right",   o.keyP2Right,   SDL_CONTROLLER_BUTTON_INVALID},
        {"down",    o.keyP2Down,    SDL_CONTROLLER_BUTTON_INVALID},
        {"jump",    o.keyP2Jump,    SDL_CONTROLLER_BUTTON_A},
        {"shoot",   o.keyP2Shoot,   SDL_CONTROLLER_BUTTON_B},
        {"melee",   o.keyP2Melee,   SDL_CONTROLLER_BUTTON_X},
        {"special", o.keyP2Special, SDL_CONTROLLER_BUTTON_Y},
        {"shield",  o.keyP2Shield,  SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
    };
    for (int i = 0; i < 8; ++i) {
        int y = ROW_Y + i * ROW_STEP;
        addWidget<KeybindWidget>(COL1_X, y, ROW_W, ROW_H, p1[i].name, p1[i].key, input, p1[i].btn);
    }
    for (int i = 0; i < 8; ++i) {
        int y = ROW_Y + i * ROW_STEP;
        addWidget<KeybindWidget>(COL2_X, y, ROW_W, ROW_H, p2[i].name, p2[i].key, input, p2[i].btn);
    }
    addWidget<Button>(SW/2-100, 780, 200, 60, "done", GREEN, WHITE, [&]{ finished = true; });
}

void ControlsScreen::handle(const SDL_Event& e) {
    for (auto& w : widgets) {
        auto* kbw = dynamic_cast<KeybindWidget*>(w.get());
        if (kbw && kbw->isListening()) {
            kbw->handle(e);
            return;
        }
    }
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:    selectedRow = (selectedRow + 7) % 8; break;
            case SDLK_DOWN:  selectedRow = (selectedRow + 1) % 8; break;
            case SDLK_LEFT:  selectedColumn = 0; break;
            case SDLK_RIGHT: selectedColumn = 1; break;
            case SDLK_RETURN: {
                int idx = selectedColumn * 8 + selectedRow;
                if (idx >= 0 && idx < (int)widgets.size()) {
                    auto* kbw = dynamic_cast<KeybindWidget*>(widgets[idx].get());
                    if (kbw) kbw->startListening();
                    return;
                }
                break;
            }
            case SDLK_ESCAPE: finished = true; break;
        }
    }
    Screen::handle(e);
}

void ControlsScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, {20,20,20,200});
    Renderer::renderText(r, titleFont, "controls", SW/2-100, 60, WHITE);
    Renderer::renderText(r, font, "player 1", COL1_X, ROW_Y-50, Color{100,149,237,255});
    Renderer::renderText(r, font, "player 2", COL2_X, ROW_Y-50, Color{255,80,80,255});
    Renderer::renderText(r, font, "click a row, then press any key; esc to cancel", SW/2-280, 720, {140,140,140,255});
    for (size_t i = 0; i < widgets.size(); ++i) {
        widgets[i]->draw(r, font);
        int row = i % 8, col = i / 8;
        if (col == selectedColumn && row == selectedRow) {
            Renderer::outlineRect(r, widgets[i]->getX(), widgets[i]->getY(), widgets[i]->getW(), widgets[i]->getH(), WHITE, 3);
        }
    }
}

bool ControlsScreen::isTransparent() const {
    return true;
}