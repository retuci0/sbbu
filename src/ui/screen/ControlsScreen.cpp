#include "ControlsScreen.h"

#include "../widget/Button.h"
#include "../widget/KeybindWidget.h"
#include "../../misc/Common.h"
#include "../../misc/Renderer.h"


static constexpr int COL1_X  = 250,  COL2_X  = 1050;
static constexpr int ROW_Y   = 160,  ROW_STEP = 70;
static constexpr int ROW_W   = 600,  ROW_H   = 55;


ControlsScreen::ControlsScreen(SDL_Renderer* r, TTF_Font* titleFont, TTF_Font* font, Options& o)
    : titleFont(titleFont), font(font)
{
    struct Row { const char* name; SDL_KeyCode& key; };

    // P1 bindings
    Row p1[] = {
        {"left",    o.keyP1Left},
        {"right",   o.keyP1Right},
        {"down",    o.keyP1Down},
        {"jump",    o.keyP1Jump},
        {"shoot",   o.keyP1Shoot},
        {"melee",   o.keyP1Melee},
        {"special", o.keyP1Special},
    };
    // P2 bindings
    Row p2[] = {
        {"left",    o.keyP2Left},
        {"right",   o.keyP2Right},
        {"down",    o.keyP2Down},
        {"jump",    o.keyP2Jump},
        {"shoot",   o.keyP2Shoot},
        {"melee",   o.keyP2Melee},
        {"special", o.keyP2Special},
    };

    for (int i = 0; i < 7; ++i) {
        int y = ROW_Y + i * ROW_STEP;
        addWidget<KeybindWidget>(COL1_X, y, ROW_W, ROW_H, p1[i].name, p1[i].key);
        addWidget<KeybindWidget>(COL2_X, y, ROW_W, ROW_H, p2[i].name, p2[i].key);
    }

    addWidget<Button>(SW/2 - 100, 680, 200, 60, "done",
        Color{50, 180, 50, 255}, WHITE, [&]{ finished = true; });
}

void ControlsScreen::handle(const SDL_Event& e) {
    for (auto& w : widgets) {
        auto* kbw = dynamic_cast<KeybindWidget*>(w.get());
        if (kbw && kbw->isListening()) {
            kbw->handle(e);
            return;
        }
    }
    Screen::handle(e);
}

void ControlsScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, {20, 20, 20, 255});
    Renderer::renderText(r, titleFont, "controls", SW/2 - 100, 60, WHITE);

    Renderer::renderText(r, font, "player 1", COL1_X, ROW_Y - 50, Color{100, 149, 237, 255});
    Renderer::renderText(r, font, "player 2", COL2_X, ROW_Y - 50, Color{255, 80, 80, 255});

    Renderer::renderText(r, font, "click a row, then press any key; esc to cancel",
        SW/2 - 280, 630, {140, 140, 140, 255});

    drawWidgets(r, font);
}
