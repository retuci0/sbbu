#pragma once

#include "ui/Screen.h"

#include "core/Options.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


enum class PauseActionResult {
    RESUME,
    QUIT, 
    RESTART, 
    CHANGE_VOLUME, 
    CHANGE_CONTROLS,
    SETTINGS
};

class PauseScreen : public Screen {
public:
    PauseScreen(const Options& options);
    void render(SDL_Renderer* renderer) override;
    void handle(const SDL_Event& event) override;
    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    PauseActionResult getResult() const { return result; }

private:
    Options options;
    bool finished = false;
    PauseActionResult result = PauseActionResult::RESUME;
    int selectedIndex = 0;
};