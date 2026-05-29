#pragma once

#include "ui/Screen.h"
#include "net/Network.h"

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
    RemoteSetupScreen();
    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* r) override;
    bool isFinished() const { return finished; }
    bool shouldGoBack() const { return goBack; }
    RemoteSetupResult takeResult() { return std::move(result); }
    
private:
    bool finished = false;
    RemoteSetupResult result;
    std::string ipInput = "127.0.0.1";
    std::string portInput = "60789";
    int activeField = 0;
    bool isHost = true;
    bool connecting = false;
    bool goBack = false;
    std::string statusMsg;
    int selectedWidget = 0;

    void tryConnect();
};