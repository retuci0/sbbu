#pragma once

#include "ui/Screen.h"

#include "misc/Characters.h"
#include "misc/Color.h"
#include "misc/Common.h"

#include <SDL2/SDL_ttf.h>

#include <array>
#include <string>


struct CharacterSelectionResult {
    const Character* char1;
    const Character* char2;
    std::string name1;
    std::string name2;
    Color color1;
    Color color2;
};

class CharacterSelectionScreen : public Screen {
public:
    CharacterSelectionScreen(const std::array<const Character*, CHARACTER_NUM>&,
                             const std::string& defaultName1, const Character* defaultChar1,
                             const std::string& defaultName2, const Character* defaultChar2);

    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    CharacterSelectionResult getResult() const { return result; }
    bool shouldGoBack() const { return goBack; }

private:
    std::array<const Character*, CHARACTER_NUM> chars;

    int selectedChar1 = 0, selectedChar2 = 0;
    std::string name1, name2;
    Color color1{}, color2{};
    int activeField = 0;    // 0 = none, 1 = name1, 2 = name2
    bool nameError = false;
    bool finished  = false;
    CharacterSelectionResult result;
    bool goBack = false;

    int  findIdx(const Character* ch) const;
    void pickColorFor(int player);  // 1 or 2
    void setDefaultColors();
    void tryStart();  // shared confirm logic
};
