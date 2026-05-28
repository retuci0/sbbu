#pragma once

#include "../Screen.h"
#include <SDL2/SDL_render.h>


enum class MultiplayerModeResult { 
    LOCAL, 
    ONLINE 
};

class TitleScreen : public Screen {
public:
    TitleScreen();
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r) override;
    bool isFinished() const { return finished; }
    MultiplayerModeResult getResult() const { return result; }
    
private:
    SDL_Texture* bg;
    bool finished = false;
    MultiplayerModeResult result = MultiplayerModeResult::LOCAL;
    int selectedIndex = 0;
};
