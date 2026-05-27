#include "CharacterSelectionScreen.h"

#include "../widget/Button.h"

#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include "../../../inc/tinyfiledialogs.h"

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
        SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* font,
        const std::array<const Character*, CHARACTER_NUM>& chars,
        const std::string& defaultName1, const Character* defaultChar1,
        const std::string& defaultName2, const Character* defaultChar2)
    : renderer(renderer), titleFont(titleFont), font(font), chars(chars),
      name1(defaultName1), name2(defaultName2)
{
    selectedChar1 = findIdx(defaultChar1);
    selectedChar2 = findIdx(defaultChar2);
    setDefaultColors();
    addWidget<Button>(START_BTN_X, START_BTN_Y, START_BTN_W, START_BTN_H,
        "start game", Color{50, 180, 50, 255}, WHITE,
        [&]{ tryStart(); });
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
    std::string n1 = name1.empty() ? "player 1" : name1;
    std::string n2 = name2.empty() ? "player 2" : name2;
    if (n1 == n2) { nameError = true; return; }
    finished = true;
    result = { chars[selectedChar1], chars[selectedChar2], n1, n2, color1, color2 };
}

void CharacterSelectionScreen::pickColorFor(int player) {
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
}

void CharacterSelectionScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    if (e.type == SDL_QUIT) {
        finished = true;
        return;
    }

    // navigation
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_RETURN:
                tryStart();
                break;
            case SDLK_BACKSPACE:
                nameError = false;
                if (activeField == 1 && !name1.empty()) name1.pop_back();
                if (activeField == 2 && !name2.empty()) name2.pop_back();
                break;
            case SDLK_LEFT:
                if (activeField == 0) activeField = 1;
                else if (activeField == 1) activeField = 2;
                else if (activeField == 2) activeField = 1;
                break;
            case SDLK_RIGHT:
                if (activeField == 0) activeField = 2;
                else if (activeField == 1) activeField = 2;
                else if (activeField == 2) activeField = 1;
                break;
            case SDLK_UP:
            case SDLK_DOWN: {
                int delta = (e.key.keysym.sym == SDLK_UP) ? -1 : 1;
                int& selected = (activeField == 1) ? selectedChar1 : selectedChar2;
                int newIdx = selected + delta;
                int first = -1, last = -1;
                for (int i = 0; i < CHARACTER_NUM; ++i) {
                    if (chars[i] && chars[i]->loaded) {
                        if (first == -1) first = i;
                        last = i;
                    }
                }
                if (newIdx < first) newIdx = last;
                if (newIdx > last) newIdx = first;
                selected = newIdx;
                break;
            }
        }
    }

    // text input
    if (e.type == SDL_TEXTINPUT) {
        nameError = false;
        if (activeField == 1 && name1.size() < 16) name1 += e.text.text;
        if (activeField == 2 && name2.size() < 16) name2 += e.text.text;
    }

    // mouse
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        const int mx = e.button.x, my = e.button.y;
        for (int i = 0; i < (int)chars.size(); ++i) {
            if (!chars[i] || !chars[i]->loaded) continue;
            int col = i / 4, row = i % 4;
            int y  = ROW_START_Y + row * ROW_STEP;
            int x1 = COL1_X + col * (BOX_W + 20);
            int x2 = COL2_X + col * (BOX_W + 20);
            if (pointInRect(mx, my, SDL_Rect{x1, y, BOX_W, BOX_H})) selectedChar1 = i;
            if (pointInRect(mx, my, SDL_Rect{x2, y, BOX_W, BOX_H})) selectedChar2 = i;
        }
        const SDL_Rect nameField1 = {NAME_BOX_X1, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H};
        const SDL_Rect nameField2 = {NAME_BOX_X2, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H};
        if      (pointInRect(mx, my, nameField1)) activeField = 1;
        else if (pointInRect(mx, my, nameField2)) activeField = 2;
        else                                       activeField = 0;
        if (pointInRect(mx, my, SDL_Rect{COL1_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H})) pickColorFor(1);
        if (pointInRect(mx, my, SDL_Rect{COL2_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H})) pickColorFor(2);
    }
}

void CharacterSelectionScreen::render(SDL_Renderer* renderer) {
    Renderer::fillRect(renderer, 0, 0, SW, SH, {30, 30, 30, 255});
    Renderer::renderText(renderer, titleFont, "character selection", 650, 60, WHITE);
    Renderer::renderText(renderer, font, "player 1", COL1_X, 150, WHITE);
    Renderer::renderText(renderer, font, "player 2", COL2_X, 150, WHITE);

    for (int i = 0; i < (int)chars.size(); ++i) {
        if (!chars[i] || !chars[i]->loaded) continue;

        int col = i / 4, row = i % 4;
        int y  = ROW_START_Y + row * ROW_STEP;
        int x1 = COL1_X + col * (BOX_W + 20);
        int x2 = COL2_X + col * (BOX_W + 20);
        const std::string charName = chars[i]->stats.name;

        SDL_Color bg1 = (selectedChar1 == i) ? SDL_Color{80, 150, 80, 255} : SDL_Color{60, 60, 60, 255};
        Renderer::renderButton(renderer, font, charName, x1, y, BOX_W, BOX_H, bg1, WHITE);

        SDL_Color bg2 = (selectedChar2 == i) ? SDL_Color{80, 150, 80, 255} : SDL_Color{60, 60, 60, 255};
        Renderer::renderButton(renderer, font, charName, x2, y, BOX_W, BOX_H, bg2, WHITE);
    }

    // character icons
    if (chars[selectedChar1] && chars[selectedChar1]->icon) {
        SDL_Rect r = {300, 130, 125, 57};
        SDL_RenderCopy(renderer, chars[selectedChar1]->icon, nullptr, &r);
    }
    if (chars[selectedChar2] && chars[selectedChar2]->icon) {
        SDL_Rect r = {1250, 130, 125, 57};
        SDL_RenderCopy(renderer, chars[selectedChar2]->icon, nullptr, &r);
    }

    // name fields
    Renderer::renderText(renderer, font, "enter name:", COL1_X, NAME_BOX_Y - 40, WHITE);
    Renderer::renderText(renderer, font, "enter name:", COL2_X, NAME_BOX_Y - 40, WHITE);
    SDL_Color nameBg1 = (activeField == 1) ? SDL_Color{100, 100, 180, 255} : SDL_Color{60, 60, 60, 255};
    SDL_Color nameBg2 = (activeField == 2) ? SDL_Color{100, 100, 180, 255} : SDL_Color{60, 60, 60, 255};
    Renderer::renderButton(renderer, font, name1, NAME_BOX_X1, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, nameBg1, WHITE);
    Renderer::renderButton(renderer, font, name2, NAME_BOX_X2, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, nameBg2, WHITE);

    // color picker buttons
    Renderer::fillRect(renderer, COL1_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H, color1);
    Renderer::fillRect(renderer, COL2_X, COLOR_BTN_Y, COLOR_BTN_W, COLOR_BTN_H, color2);
    Renderer::renderText(renderer, font, "pick color", COL1_X + 10, COLOR_BTN_Y + 12, BLACK);
    Renderer::renderText(renderer, font, "pick color", COL2_X + 10, COLOR_BTN_Y + 12, BLACK);

    // name conflict error
    if (nameError) {
        Renderer::renderText(renderer, font, "player names must be different!",
            START_BTN_X - 60, START_BTN_Y - 40, {255, 80, 80, 255});
    }

    drawWidgets(renderer, font);
}
