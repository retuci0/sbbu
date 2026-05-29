#pragma once

#include "ui/Screen.h"

#include <SDL2/SDL_render.h>


enum class TitleScreenResult { 
    LOCAL, 
    ONLINE,
    QUIT
};

class TitleScreen : public Screen {
public:
    TitleScreen();
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r) override;
    bool isFinished() const { return finished; }
    TitleScreenResult getResult() const { return result; }
    
private:
    SDL_Texture* bg;
    bool finished = false;
    TitleScreenResult result = TitleScreenResult::QUIT;
    int selectedIndex = 0;
};
