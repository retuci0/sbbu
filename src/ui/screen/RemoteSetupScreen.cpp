#include "RemoteSetupScreen.h"

#include "../widget/Button.h"
#include "../../misc/Renderer.h"
#include "../../misc/Common.h"


RemoteSetupScreen::RemoteSetupScreen(SDL_Renderer* r, TTF_Font* tf, TTF_Font* f)
    : renderer(r), titleFont(tf), font(f) {
    addWidget<Button>(SW/2-300, SH-150, 250, 70, "HOST",
                      Color{60,100,60}, WHITE, [&]{ isHost = true; tryConnect(); });
    addWidget<Button>(SW/2+50,  SH-150, 250, 70, "CLIENT",
                      Color{60,60,100}, WHITE, [&]{ isHost = false; tryConnect(); });
}

void RemoteSetupScreen::tryConnect() {
    connecting = true;
    uint16_t port = static_cast<uint16_t>(std::stoi(portInput));
    if (isHost) {
        result.network = Network::createHost(port);
        if (result.network && result.network->isConnected()) {
            result.role = RemoteSetupRole::HOST;
            finished = true;
        } else statusMsg = "failed to host on port " + portInput;
    } else {
        result.network = Network::createClient(ipInput.c_str(), port);
        if (result.network) {
            result.role = RemoteSetupRole::CLIENT;
            finished = true;
        } else statusMsg = "couldn't connect to " + ipInput + ":" + portInput;
    }
    connecting = false;
}

void RemoteSetupScreen::handle(const SDL_Event& e) {
    Screen::handle(e);
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE) {
            if (activeField == 1 && !ipInput.empty()) ipInput.pop_back();
            if (activeField == 2 && !portInput.empty()) portInput.pop_back();
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

void RemoteSetupScreen::render(SDL_Renderer* r, TTF_Font* f) {
    Renderer::fillRect(r, 0, 0, SW, SH, Color{20,20,40});
    Renderer::renderText(r, titleFont, "Online Setup", SW/2-120, 100, WHITE);

    Renderer::renderText(r, font, "Server IP:", SW/2-200, SH/2-70, WHITE);
    SDL_Color ipBg = (activeField == 1) ? Color{ 100,100,180 }.toSdlColor() : Color{ 60,60,60 }.toSdlColor();
    Renderer::renderButton(r, font, ipInput, SW/2-200, SH/2-40, 400, 50, ipBg, WHITE);

    Renderer::renderText(r, font, "Port:", SW/2-200, SH/2, WHITE);
    SDL_Color portBg = (activeField == 2) ? Color{ 100,100,180 }.toSdlColor() : Color{ 60,60,60 }.toSdlColor();
    Renderer::renderButton(r, font, portInput, SW/2-200, SH/2+30, 400, 50, portBg, WHITE);

    if (!statusMsg.empty()) {
        Renderer::renderText(r, font, statusMsg, SW/2-150, SH/2+120, Color{255,100,100});
    }

    drawWidgets(r, f);
}