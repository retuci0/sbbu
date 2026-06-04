#pragma once

#include "ui/Screen.h"

#include "core/Options.h"

#include <SDL2/SDL_ttf.h>


class InputHandler;

class ControlsScreen : public Screen {
public:
    ControlsScreen(Options& options, InputHandler& input);
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;
    bool isFinished() const { return finished; }
    bool isTransparent() const override;
    
private:
    bool finished = false;
    int selectedRow = 0, selectedColumn = 0;
};