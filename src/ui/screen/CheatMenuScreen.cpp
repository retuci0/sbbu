#include "ui/screen/CheatMenuScreen.h"

#include "core/Game.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include "ui/widget/ButtonWidget.h"
#include "ui/widget/FieldWidget.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


static constexpr int COL0_X  = SW / 2 - 460;
static constexpr int COL1_X  = SW / 2 - 10;
static constexpr int BTN_W   = 440;
static constexpr int BTN_H   = 70;
static constexpr int ROW_GAP = 20;
static constexpr int BTN_ROW_Y = 340;

static constexpr int FIELD_Y   = 220;
static constexpr int FIELD_W   = 200;
static constexpr int FIELD_H   = 50;
static constexpr int FIELD_GAP = 60;


CheatMenuScreen::CheatMenuScreen(Game* game) : Screen() {
    xField  = addWidget<FieldWidget>(COL0_X,              FIELD_Y, FIELD_W, FIELD_H, font, "500");
    yField  = addWidget<FieldWidget>(COL0_X + FIELD_W + FIELD_GAP, FIELD_Y, FIELD_W, FIELD_H, font, "200");
    hpField = addWidget<FieldWidget>(COL1_X + BTN_W - FIELD_W,     FIELD_Y, FIELD_W, FIELD_H, font, "100");

    addWidget<ButtonWidget>(COL0_X, BTN_ROW_Y, BTN_W, BTN_H, "tp p1", GREEN, WHITE,
        [&, game]{
            auto x = getX(); auto y = getY();
            if (x) game->player1.rect.x = *x;
            if (y) game->player1.rect.y = *y;
        }
    );
    addWidget<ButtonWidget>(COL1_X, BTN_ROW_Y, BTN_W, BTN_H, "tp p2", GREEN, WHITE,
        [&, game]{
            auto x = getX(); auto y = getY();
            if (x) game->player2.rect.x = *x;
            if (y) game->player2.rect.y = *y;
        }
    );

    addWidget<ButtonWidget>(COL0_X, BTN_ROW_Y + (BTN_H + ROW_GAP), BTN_W, BTN_H, "heal p1", GREEN, WHITE,
        [&, game]{
            auto hp = getHp();
            if (hp) game->player1.hp = std::min(*hp, game->player1.character.stats.health);
        }
    );
    addWidget<ButtonWidget>(COL1_X, BTN_ROW_Y + (BTN_H + ROW_GAP), BTN_W, BTN_H, "heal p2", GREEN, WHITE,
        [&, game]{
            auto hp = getHp();
            if (hp) game->player2.hp = std::min(*hp, game->player2.character.stats.health);
        }
    );

    addWidget<ButtonWidget>(COL0_X, BTN_ROW_Y + 2 * (BTN_H + ROW_GAP), BTN_W, BTN_H, "kill p1", GREEN, WHITE,
        [&, game]{ game->player1.hp = 0; }
    );
    addWidget<ButtonWidget>(COL1_X, BTN_ROW_Y + 2 * (BTN_H + ROW_GAP), BTN_W, BTN_H, "kill p2", GREEN, WHITE,
        [&, game]{ game->player2.hp = 0; }
    );

    addWidget<ButtonWidget>(SW / 2 - BTN_W / 2, BTN_ROW_Y + 3 * (BTN_H + ROW_GAP), BTN_W, BTN_H, "spawn random item", GREEN, WHITE,
        [game]{ game->trySpawnItem(); }
    );
}


static constexpr int FIRST_BTN = 3;
static constexpr int BTN_COLS  = 2;

void CheatMenuScreen::handle(const SDL_Event& event) {
    Screen::handle(event);
    if (event.type == SDL_KEYDOWN) {
        error.clear();
        const int btnCount = static_cast<int>(widgets.size()) - FIRST_BTN;
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                requestTransition(ScreenAction::POP);
                return;
            case SDLK_UP:
                if (selectedIndex >= BTN_COLS) selectedIndex -= BTN_COLS;
                break;
            case SDLK_DOWN:
                if (selectedIndex + BTN_COLS < btnCount) selectedIndex += BTN_COLS;
                break;
            case SDLK_LEFT:
                if (selectedIndex % BTN_COLS == 1) selectedIndex--;
                break;
            case SDLK_RIGHT:
                if (selectedIndex % BTN_COLS == 0 && selectedIndex + 1 < btnCount) selectedIndex++;
                break;
            case SDLK_RETURN: {
                auto* btn = dynamic_cast<ButtonWidget*>(widgets[FIRST_BTN + selectedIndex].get());
                if (btn) btn->activate();
                return;
            }
        }
    }
}

void CheatMenuScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, { 0, 0, 0, 160 });
    Renderer::renderText(renderer, titleFont, "cheat menu", SW / 2 - 130, 100, WHITE);

    // draw labels
    Renderer::renderText(renderer, font, "x:",  COL0_X,                          FIELD_Y - 36, WHITE);
    Renderer::renderText(renderer, font, "y:",  COL0_X + FIELD_W + FIELD_GAP,    FIELD_Y - 36, WHITE);
    Renderer::renderText(renderer, font, "hp:", COL1_X + BTN_W - FIELD_W,        FIELD_Y - 36, WHITE);

    // draw fields
    for (int i = 0; i < FIRST_BTN; ++i) {
        widgets[i]->draw(renderer, font);
    }

    // draw buttons
    const int btnCount = static_cast<int>(widgets.size()) - FIRST_BTN;
    for (int i = 0; i < btnCount; ++i) {
        auto* btn = dynamic_cast<ButtonWidget*>(widgets[FIRST_BTN + i].get());
        if (!btn) continue;
        btn->draw(renderer, font);
        if (i == selectedIndex) {
            Renderer::outlineRect(renderer, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
        }
    }

    // draw error
    if (!error.empty()) {
        int w;
        TTF_SizeText(font, error.c_str(), &w, nullptr);
        Renderer::renderText(renderer, smallFont, error, (SW - w) / 2, 10, RED);
    }
}