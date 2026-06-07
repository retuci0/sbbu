#include "ui/screen/RemoteSetupScreen.h"

#include "ui/widget/ButtonWidget.h"
#include "ui/widget/FieldWidget.h"

#include "misc/Renderer.h"
#include "misc/Common.h"

#include <SDL2/SDL_net.h>

#include <iostream>
#include <ostream>
#include <string>


RemoteSetupScreen::RemoteSetupScreen() : Screen() {
    addWidget<ButtonWidget>(4, 4, 64, 64, "<", GREEN, WHITE, [&]{ goBack(); });
    addWidget<ButtonWidget>(SW/2-300, SH-150, 250, 70, "HOST",   GREEN, WHITE, [&]{ isHost = true;  tryConnect(); });
    addWidget<ButtonWidget>(SW/2+50,  SH-150, 250, 70, "CLIENT", GREEN, WHITE, [&]{ isHost = false; tryConnect(); });
    ipField   = addWidget<FieldWidget>(SW/2-200, SH/2-40,  400, 50, font, "127.0.0.1");
    portField = addWidget<FieldWidget>(SW/2-200, SH/2+30,  400, 50, font, "60789");
}

void RemoteSetupScreen::tryConnect() {
    connecting = true;
    uint16_t port = static_cast<uint16_t>(std::stoi(portField->getText()));
    const std::string ip = ipField->getText();
    if (isHost) {
        result.network = Network::createHost(port);
        if (result.network) { result.role = RemoteSetupRole::HOST; finished = true; }
        else { statusMsg = "failed to host on port " + portField->getText(); std::cout << SDLNet_GetError() << std::endl; }
    } else {
        result.network = Network::createClient(ip.c_str(), port);
        if (result.network) { result.role = RemoteSetupRole::CLIENT; finished = true; }
        else { statusMsg = "couldn't connect to " + ip + ":" + portField->getText(); std::cout << SDLNet_GetError() << std::endl; }
    }
    connecting = false;
}

void RemoteSetupScreen::resetFinished() {
    finished = false;
    result = {};
}

void RemoteSetupScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_RETURN:
                switch (selectedWidget) {
                    case 0: goBack(); break;
                    case 3: isHost = true;  tryConnect(); break;
                    case 4: isHost = false; tryConnect(); break;
                }
                break;
            case SDLK_UP:
            case SDLK_DOWN:
                selectedWidget = (selectedWidget + (e.key.keysym.sym == SDLK_UP ? -1 : 1) + 5) % 5;
                break;
            case SDLK_ESCAPE:
                goBack();
                break;
        }
    }
}

void RemoteSetupScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20, 20, 40});
    Renderer::renderText(r, titleFont, "online setup", SW/2-120, 100, WHITE);

    Renderer::renderText(r, font, "server IP:", SW/2-200, SH/2-70, WHITE);
    Renderer::renderText(r, font, "port:",      SW/2-200, SH/2,    WHITE);

    if (!statusMsg.empty()) {
        Renderer::renderText(r, font, statusMsg, SW/2-150, SH/2+120, Color{255, 100, 100});
    }

    if (selectedWidget == 0) {
        auto* btn = dynamic_cast<ButtonWidget*>(widgets[0].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    } else if (selectedWidget == 1) {
        Renderer::outlineRect(r, SW/2-200, SH/2-40, 400, 50, WHITE, 3);
    } else if (selectedWidget == 2) {
        Renderer::outlineRect(r, SW/2-200, SH/2+30, 400, 50, WHITE, 3);
    } else if (selectedWidget == 3) {
        auto* btn = dynamic_cast<ButtonWidget*>(widgets[1].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    } else if (selectedWidget == 4) {
        auto* btn = dynamic_cast<ButtonWidget*>(widgets[2].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    }

    drawWidgets(r, font);
}