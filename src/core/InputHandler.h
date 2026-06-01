#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include <unordered_map>
#include <array>


constexpr int MAX_CONTROLLERS = 2;
constexpr float AXIS_THRESHOLD = 0.2f;

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

    // controller queries (ctrl = 0 for P1, 1 for P2)
    bool isDown(SDL_GameControllerButton button, int ctrl = 0) const;
    bool isPressed(SDL_GameControllerButton button, int ctrl = 0) const;
    bool isReleased(SDL_GameControllerButton button, int ctrl = 0) const;
    Sint16 getControllerAxis(SDL_GameControllerAxis axis, int ctrl = 0) const;
    float getNormalizedAxis(SDL_GameControllerAxis axis, int ctrl = 0, int deadzone = 8000) const;

    bool isControllerConnected(int ctrl = 0) const { return controllers[ctrl] != nullptr; }
    int  numControllers() const;

    // returns which slot {0,1} a joystick instance ID belongs to, or -1 if not found
    int getControllerSlot(SDL_JoystickID instanceId) const;

protected:
    virtual void onKey(SDL_Keycode key, KeyAction action) {}
    virtual void onControllerButton(SDL_GameControllerButton button, ControllerButtonAction action, int ctrl) {}

private:
    std::unordered_map<SDL_Keycode, KeyState> keys;

    std::array<SDL_GameController*, MAX_CONTROLLERS> controllers = { nullptr, nullptr };
    std::array<SDL_JoystickID, MAX_CONTROLLERS>      controllerInstanceIds = { -1, -1 };
    std::array<std::unordered_map<SDL_GameControllerButton, ControllerButtonState>, MAX_CONTROLLERS> controllerButtons;
    std::array<std::unordered_map<SDL_GameControllerAxis,   Sint16>,                MAX_CONTROLLERS> axisValues;

    int  findFreeSlot() const;
    void openController(int deviceIndex);
    void closeControllerSlot(int slot);
};