#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <unordered_map>


struct KeyState {
    bool down = false;
    bool pressed = false;
    bool released = false;
    bool repeated = false;
};

struct ControllerButtonState {
    bool down = false;
    bool pressed = false;
    bool released = false;
};

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

enum class ControllerButtonAction { 
    PRESS, 
    RELEASE 
};

class InputHandler {
public:
    virtual ~InputHandler() = default;

    void init();

    void shutdown();

    void beginFrame();

    void processEvent(const SDL_Event& e);

    // keyboard queries
    bool isDown(SDL_Keycode key) const;
    bool isPressed(SDL_Keycode key) const;
    bool isReleased(SDL_Keycode key) const;
    bool isRepeated(SDL_Keycode key) const;

    // controller queries
    bool isDown(SDL_GameControllerButton button) const;
    bool isPressed(SDL_GameControllerButton button) const;
    bool isReleased(SDL_GameControllerButton button) const;
    Sint16 getControllerAxis(SDL_GameControllerAxis axis) const;
    float getNormalizedAxis(SDL_GameControllerAxis axis, int deadzone = 8000) const;

protected:
    virtual void onKey(SDL_Keycode key, KeyAction action) {}
    virtual void onControllerButton(SDL_GameControllerButton button, ControllerButtonAction action) {}

private:
    // state
    std::unordered_map<SDL_Keycode, KeyState> keys;
    SDL_GameController* controller = nullptr;
    std::unordered_map<SDL_GameControllerButton, ControllerButtonState> controllerButtons;
    std::unordered_map<SDL_GameControllerAxis, Sint16> axisValues;

    void openController(int deviceIndex);
    void closeController();
};