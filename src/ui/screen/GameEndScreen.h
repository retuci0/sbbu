#pragma once

#include "ui/Screen.h"

#include <SDL2/SDL_ttf.h>

#include <string>


enum class GameEndActionResult {
    TITLE,
    QUIT
};

class GameEndScreen : public Screen {
public:
    GameEndScreen(const std::string& title, const std::string& details);

    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished() const { return finished; }
    GameEndActionResult getResult() const { return result; }

private:
    std::string title;
    std::string details;
    bool finished = false;
    GameEndActionResult result = GameEndActionResult::TITLE;
    int selectedIndex = 0;
};