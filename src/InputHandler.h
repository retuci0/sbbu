#pragma once

#include <SDL2/SDL.h>

#include <unordered_map>


struct Input {
    bool jumpP1    = false;
    bool shootP1   = false;
    bool meleeP1   = false;
    bool specialP1 = false;
    bool jumpP2    = false;
    bool shootP2   = false;
    bool meleeP2   = false;
    bool specialP2 = false;
};

enum class KeyAction {
    PRESS,
    REPEAT,
    RELEASE
};

class InputHandler {
public:
    virtual ~InputHandler() = default;

    void beginFrame();
    void processEvent(const SDL_Event& ev);

    bool isDown(SDL_Keycode key) const;
    bool isPressed(SDL_Keycode key) const;
    bool isReleased(SDL_Keycode key) const;
    bool isRepeated(SDL_Keycode key) const;

protected:
    virtual void onKey(SDL_Keycode key, KeyAction action) {}

private:
    struct KeyState {
        bool down = false;
        bool pressed  = false;
        bool released = false;
        bool repeated = false;
    };

    std::unordered_map<SDL_Keycode, KeyState> keys;
};