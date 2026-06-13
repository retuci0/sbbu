#include "ui/screen/CharacterSelectionScreen.h"

#include "ui/widget/ButtonWidget.h"
#include "ui/widget/FieldWidget.h"

#include "misc/Common.h"
#include "misc/Renderer.h"

#include "tinyfiledialogs.h"

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>

#include <array>


static constexpr int BOX_W = 360, BOX_H = 100;
static constexpr int COL1_X = 100, COL2_X = 1050;
static constexpr int ROW_START_Y = 200, ROW_STEP = 120;
static constexpr int NAME_BOX_X1 = 100,  NAME_BOX_X2 = 1050;
static constexpr int NAME_BOX_Y  = 720,  NAME_BOX_W  = 360, NAME_BOX_H = 60;
static constexpr int START_BTN_X = 760, START_BTN_Y = 870;
static constexpr int START_BTN_W = 400, START_BTN_H = 80;
static constexpr int COLOR_BTN_W = 160, COLOR_BTN_H = 50;
static constexpr int COLOR_BTN_Y = NAME_BOX_Y + 80;

CharacterSelectionScreen::CharacterSelectionScreen(
        const std::array<const Character*, CHARACTER_NUM>& chars,
        const std::string& defaultName1, const Character* defaultChar1,
        const std::string& defaultName2, const Character* defaultChar2)
    : chars(chars), Screen()
{
    selectedChar1 = findIdx(defaultChar1);
    selectedChar2 = findIdx(defaultChar2);
    setDefaultColors();
    addWidget<ButtonWidget>(START_BTN_X, START_BTN_Y, START_BTN_W, START_BTN_H,
        "start game", GREEN, WHITE,
        [&]{ tryStart(); });
    addWidget<ButtonWidget>(4, 4, 64, 64, "<", GREEN, WHITE,
        [&]{ goBack(); });
    nameField1 = addWidget<FieldWidget>(NAME_BOX_X1, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, font, defaultName1);
    nameField2 = addWidget<FieldWidget>(NAME_BOX_X2, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, font, defaultName2);
}

int CharacterSelectionScreen::findIdx(const Character* ch) const {
    for (int i = 0; i < CHARACTER_NUM; ++i)
        if (chars[i] && chars[i]->loaded && chars[i] == ch) return i;
    return 0;
}

void CharacterSelectionScreen::setDefaultColors() {
    color1 = {100, 149, 237, 230};
    color2 = {255,  80,  80, 230};
}

void CharacterSelectionScreen::tryStart() {
    std::string n1 = nameField1->getText().empty() ? "player 1" : nameField1->getText();
    std::string n2 = nameField2->getText().empty() ? "player 2" : nameField2->getText();
    if (n1 == n2) { error = "player names must be different!"; return; }
    finished = true;
    result   = { chars[selectedChar1], chars[selectedChar2], n1, n2, color1, color2 };
}

void CharacterSelectionScreen::pickColorFor(int player) {
#ifdef __EMSCRIPTEN__
    error = "color picking not supported in web!";
#else
    const char* defaultHex = (player == 1) ? "#6495ED" : "#FF5050";
    unsigned char defaultRgb[3] = {
        (player == 1) ? static_cast<unsigned char>(100) : static_cast<unsigned char>(255),
        (player == 1) ? static_cast<unsigned char>(149) : static_cast<unsigned char>(80),
        (player == 1) ? static_cast<unsigned char>(237) : static_cast<unsigned char>(80)
    };
    unsigned char resultRgb[3];
    const char* hex = tinyfd_colorChooser(
        (player == 1) ? "pick player 1 color" : "pick player 2 color",
        defaultHex, defaultRgb, resultRgb
    );
    if (hex) {
        SDL_Color newColor = { resultRgb[0], resultRgb[1], resultRgb[2], 230 };
        if (player == 1) color1 = newColor;
        else             color2 = newColor;
    }
#endif
}

void CharacterSelectionScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    if (e.type == SDL_QUIT) {
        finished = true;
        return;
    }

    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_RETURN:
                tryStart();
                break;
            case SDLK_UP:
            case SDLK_DOWN: {
                int delta = (e.key.keysym.sym == SDLK_UP) ? -1 : 1;
                int& selected = selectedChar1;  // default to p1
                int first = -1, last = -1;
                for (int i = 0; i < CHARACTER_NUM; ++i) {
                    if (chars[i] && chars[i]->loaded) {
                        if (first == -1) first = i;
                        last = i;
                    }
                }
                int newIdx = selected + delta;
                if (newIdx < first) newIdx = last;
                if (newIdx > last)  newIdx = first;
                selected = newIdx;
                break;
            }
            case SDLK_ESCAPE:
                goBack();
                break;
        }
        error.clear();
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        const int mx = e.button.x, my = e.button.y;
        for (int i = 0; i < static_cast<int>(chars.size()); ++i) {
            if (!chars[i] || !chars[i]->loaded) continue;
            int col = i / 4, row = i % 4;
            int y  = ROW_START_Y + row * ROW_STEP;
            int x1 = COL1_X + col * (BOX_W + 20);
            int x2 = COL2_X + col * (BOX_W + 20);
            if (pointInRect(mx, my, SDL_Rect{x1, y, BOX_W, BOX_H})) selectedChar1 = i;
            if (pointInRect(mx, my, SDL_Rect{x2, y, BOX_W, BOX_H})) selectedChar2 = i;
        }
        if (pointInRect(mx, my, SDL_Rect{COL1_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H})) {
            pickColorFor(1);
        }
        if (pointInRect(mx, my, SDL_Rect{COL2_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H})) {
            pickColorFor(2);
        }
    }
}

void CharacterSelectionScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {30, 30, 30, 255});
    Renderer::renderText(renderer, titleFont, "character selection", 650, 60, WHITE);
    Renderer::renderText(renderer, font, "player 1", COL1_X, 150, WHITE);
    Renderer::renderText(renderer, font, "player 2", COL2_X, 150, WHITE);

    for (int i = 0; i < static_cast<int>(chars.size()); ++i) {
        if (!chars[i] || !chars[i]->loaded) continue;

        int col = i / 4, row = i % 4;
        int y  = ROW_START_Y + row * ROW_STEP;
        int x1 = COL1_X + col * (BOX_W + 20);
        int x2 = COL2_X + col * (BOX_W + 20);
        const std::string charName = chars[i]->stats.name;

        SDL_Color bg1 = (selectedChar1 == i) ? GREEN.brighter().toSdlColor() : GREEN.toSdlColor();
        Renderer::renderButton(renderer, font, charName, x1, y, BOX_W, BOX_H, bg1, WHITE);

        SDL_Color bg2 = (selectedChar2 == i) ? GREEN.brighter().toSdlColor() : GREEN.toSdlColor();
        Renderer::renderButton(renderer, font, charName, x2, y, BOX_W, BOX_H, bg2, WHITE);
    }

    if (chars[selectedChar1] && chars[selectedChar1]->icon) {
        SDL_Rect r = {300, 130, 125, 57};
        Renderer::drawSprite(renderer, chars[selectedChar1]->icon, &r, false);
    }
    if (chars[selectedChar2] && chars[selectedChar2]->icon) {
        SDL_Rect r = {1250, 130, 125, 57};
        Renderer::drawSprite(renderer, chars[selectedChar2]->icon, &r, false);
    }

    Renderer::renderText(renderer, font, "enter name:", COL1_X, NAME_BOX_Y - 40, WHITE);
    Renderer::renderText(renderer, font, "enter name:", COL2_X, NAME_BOX_Y - 40, WHITE);

    Renderer::fillRect(renderer, COL1_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H, color1);
    Renderer::fillRect(renderer, COL2_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H, color2);
    Renderer::renderText(renderer, font, "pick color", COL1_X + 10, COLOR_BTN_Y + 12, BLACK);
    Renderer::renderText(renderer, font, "pick color", COL2_X + 10, COLOR_BTN_Y + 12, BLACK);
    
    drawWidgets(renderer, font);

    if (!error.empty()) {
        int w; TTF_SizeText(font, error.c_str(), &w, nullptr);
        Renderer::renderText(renderer, font, error,
            (SW - w) / 2, START_BTN_Y + 40, RED);
    }
}