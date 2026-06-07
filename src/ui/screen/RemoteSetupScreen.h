#pragma once

#include "ui/Screen.h"
#include "ui/widget/FieldWidget.h"
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
    RemoteSetupResult takeResult() { return std::move(result); }
    void resetFinished();

private:
    bool finished  = false;
    bool isHost    = true;
    bool connecting = false;
    int  selectedWidget = 0;
    std::string statusMsg;
    RemoteSetupResult result;

    FieldWidget* ipField   = nullptr;
    FieldWidget* portField = nullptr;

    void tryConnect();
};