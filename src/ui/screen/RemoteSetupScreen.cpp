#include "RemoteSetupScreen.h"

#include "../widget/Button.h"
#include "../../misc/Renderer.h"
#include "../../misc/Common.h"

#include <SDL2/SDL_net.h>

#include <iostream>
#include <ostream>


RemoteSetupScreen::RemoteSetupScreen() : Screen() {
    addWidget<Button>(4, 4, 64, 64, "<", Color{60,100,60}, WHITE, [&]{ goBack = true; finished = true; });
    addWidget<Button>(SW/2-300, SH-150, 250, 70, "HOST", Color{60,100,60}, WHITE, [&]{ isHost = true; tryConnect(); });
    addWidget<Button>(SW/2+50,  SH-150, 250, 70, "CLIENT", Color{60,60,100}, WHITE, [&]{ isHost = false; tryConnect(); });
}

void RemoteSetupScreen::tryConnect() {
    connecting = true;
    uint16_t port = static_cast<uint16_t>(std::stoi(portInput));
    if (isHost) {
        result.network = Network::createHost(port);
        if (result.network) { result.role = RemoteSetupRole::HOST; finished = true; }
        else { statusMsg = "failed to host on port " + portInput; std::cout << SDLNet_GetError() << std::endl; }
    } else {
        result.network = Network::createClient(ipInput.c_str(), port);
        if (result.network) { result.role = RemoteSetupRole::CLIENT; finished = true; }
        else { statusMsg = "couldn't connect to " + ipInput + ":" + portInput; std::cout << SDLNet_GetError() << std::endl; }
    }
    connecting = false;
}

void RemoteSetupScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_DOWN:
                selectedWidget = (selectedWidget + (e.key.keysym.sym == SDLK_UP ? -1 : 1) + 5) % 5;
                break;
            case SDLK_RETURN:
                switch (selectedWidget) {
                    case 0: goBack = true; finished = true; break;
                    case 1: activeField = 1; break;
                    case 2: activeField = 2; break;
                    case 3: isHost = true; tryConnect(); break;
                    case 4: isHost = false; tryConnect(); break;
                }
                break;
            case SDLK_ESCAPE:
                goBack = true; finished = true; break;
            case SDLK_BACKSPACE:
                if (activeField == 1 && !ipInput.empty()) ipInput.pop_back();
                if (activeField == 2 && !portInput.empty()) portInput.pop_back();
                break;
        }
    }
    if (e.type == SDL_TEXTINPUT) {
        if (activeField == 1 && ipInput.size() < 30) ipInput += e.text.text;
        if (activeField == 2 && portInput.size() < 5) portInput += e.text.text;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mx = e.button.x, my = e.button.y;
        SDL_Rect ipRect = {SW/2-200, SH/2-40, 400, 50};
        SDL_Rect portRect = {SW/2-200, SH/2+30, 400, 50};
        if (pointInRect(mx, my, ipRect)) activeField = 1;
        else if (pointInRect(mx, my, portRect)) activeField = 2;
        else activeField = 0;
    }
}

void RemoteSetupScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20,20,40});
    Renderer::renderText(r, titleFont, "online setup", SW/2-120, 100, WHITE);

    Renderer::renderText(r, font, "server IP:", SW/2-200, SH/2-70, WHITE);
    SDL_Color ipBg = (activeField == 1) ? Color{100,100,180}.toSdlColor() : Color{60,60,60}.toSdlColor();
    Renderer::renderButton(r, font, ipInput, SW/2-200, SH/2-40, 400, 50, ipBg, WHITE);

    Renderer::renderText(r, font, "port:", SW/2-200, SH/2, WHITE);
    SDL_Color portBg = (activeField == 2) ? Color{100,100,180}.toSdlColor() : Color{60,60,60}.toSdlColor();
    Renderer::renderButton(r, font, portInput, SW/2-200, SH/2+30, 400, 50, portBg, WHITE);

    // status message
    if (!statusMsg.empty()) {
        Renderer::renderText(r, font, statusMsg, SW/2-150, SH/2+120, Color{255,100,100});
    }

    // highlight selected widget
    if (selectedWidget == 0) {
        auto* btn = dynamic_cast<Button*>(widgets[0].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    } else if (selectedWidget == 1)
        Renderer::outlineRect(r, SW/2-200, SH/2-40, 400, 50, WHITE, 3);
    else if (selectedWidget == 2)
        Renderer::outlineRect(r, SW/2-200, SH/2+30, 400, 50, WHITE, 3);
    else if (selectedWidget == 3) {
        auto* btn = dynamic_cast<Button*>(widgets[1].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    } else if (selectedWidget == 4) {
        auto* btn = dynamic_cast<Button*>(widgets[2].get());
        if (btn) Renderer::outlineRect(r, btn->getX(), btn->getY(), btn->getW(), btn->getH(), WHITE, 3);
    }

    drawWidgets(r, font);
}