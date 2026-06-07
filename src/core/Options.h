#pragma once

#include <SDL2/SDL_keycode.h>

#include <string>


class Options {
public:
    SDL_KeyCode keyP1Left       = SDLK_a;
    SDL_KeyCode keyP1Right      = SDLK_d;
    SDL_KeyCode keyP1Down       = SDLK_s;
    SDL_KeyCode keyP1Jump       = SDLK_SPACE;
    SDL_KeyCode keyP1Shoot      = SDLK_e;
    SDL_KeyCode keyP1Melee      = SDLK_w;
    SDL_KeyCode keyP1Special    = SDLK_q;
    SDL_KeyCode keyP1Shield     = SDLK_LSHIFT;
    SDL_KeyCode keyP1Dash       = SDLK_z;
    SDL_KeyCode keyP1Grapple    = SDLK_r;

    SDL_KeyCode keyP2Left       = SDLK_LEFT;
    SDL_KeyCode keyP2Right      = SDLK_RIGHT;
    SDL_KeyCode keyP2Down       = SDLK_DOWN;
    SDL_KeyCode keyP2Jump       = SDLK_UP;
    SDL_KeyCode keyP2Shoot      = SDLK_RCTRL;
    SDL_KeyCode keyP2Melee      = SDLK_RSHIFT;
    SDL_KeyCode keyP2Special    = SDLK_RETURN;
    SDL_KeyCode keyP2Shield     = SDLK_SLASH;
    SDL_KeyCode keyP2Dash       = SDLK_BACKSLASH;
    SDL_KeyCode keyP2Grapple    = SDLK_QUOTE;

    SDL_KeyCode keyPause        = SDLK_ESCAPE;
    SDL_KeyCode keyQuit         = SDLK_F7;
    SDL_KeyCode keyDebug        = SDLK_F3;
    SDL_KeyCode keyFullscreen   = SDLK_F11;
    SDL_KeyCode keyScreenshot   = SDLK_F2;
    SDL_KeyCode keyCheats       = SDLK_F6;

    float sfxVolume             = 1.0f;
    float musVolume             = 1.0f;

    bool vsync                  = false;
    bool fullscreen             = true;

    int fpsCap                  = -1;  // -1 = unlimited

    bool debug                  = false;

    bool particles              = true;

    void saveToFile(const std::string& filename = "config.cfg") const;
    void loadFromFile(const std::string& filename = "config.cfg");
};