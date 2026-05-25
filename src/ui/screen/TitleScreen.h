#pragma once

#include "../Screen.h"


enum class MultiplayerModeResult { 
    LOCAL, 
    ONLINE 
};

class TitleScreen : public Screen {
public:
    TitleScreen(SDL_Renderer* r, TTF_Font* titleFont, TTF_Font* font);
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r, TTF_Font* f) override;
    bool isFinished() const { return finished; }
    MultiplayerModeResult getResult() const { return result; }
    
private:
    SDL_Renderer* renderer;
    TTF_Font *titleFont, *font;
    bool finished = false;
    MultiplayerModeResult result = MultiplayerModeResult::LOCAL;
};