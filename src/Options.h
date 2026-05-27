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

    SDL_KeyCode keyP2Left       = SDLK_LEFT;
    SDL_KeyCode keyP2Right      = SDLK_RIGHT;
    SDL_KeyCode keyP2Down       = SDLK_DOWN;
    SDL_KeyCode keyP2Jump       = SDLK_UP;
    SDL_KeyCode keyP2Shoot      = SDLK_LCTRL;
    SDL_KeyCode keyP2Melee      = SDLK_LSHIFT;
    SDL_KeyCode keyP2Special    = SDLK_RETURN;

    SDL_KeyCode keyPause        = SDLK_ESCAPE;
    SDL_KeyCode keyQuit         = SDLK_0;
    SDL_KeyCode keyDebug        = SDLK_b;
    SDL_KeyCode keyFullscreen   = SDLK_F11;

    float sfxVolume             = 1.0f;
    float musVolume             = 1.0f;

    bool debug                  = false;

    void saveToFile  (const std::string& filename = "config.cfg") const;
    void loadFromFile(const std::string& filename = "config.cfg");
};