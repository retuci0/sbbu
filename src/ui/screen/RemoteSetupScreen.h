#pragma once

#include "../Screen.h"

#include "../../net/Network.h"

#include <memory>


enum class RemoteSetupRole { 
    HOST, 
    CLIENT 
};

struct RemoteSetupResult {
    std::unique_ptr<Network> network;
    RemoteSetupRole role;
};

class RemoteSetupScreen : public Screen {
public:
    RemoteSetupScreen(SDL_Renderer* r, TTF_Font* titleFont, TTF_Font* font);
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r, TTF_Font* f) override;
    bool isFinished() const { return finished; }
    RemoteSetupResult takeResult() { return std::move(result); }
    
private:
    SDL_Renderer* renderer;
    TTF_Font *titleFont, *font;
    bool finished = false;
    RemoteSetupResult result;

    std::string ipInput = "127.0.0.1";
    std::string portInput = "12345";
    int activeField = 0;
    bool isHost = true;
    bool connecting = false;
    std::string statusMsg;

    void tryConnect();
};