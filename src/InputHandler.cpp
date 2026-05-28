#include "InputHandler.h"

#include <SDL2/SDL.h>

#include <cmath>


void InputHandler::init() {
    SDL_GameControllerEventState(SDL_ENABLE);
    // open up to 2 already-connected controllers
    int n = SDL_NumJoysticks();
    for (int i = 0; i < n && numControllers() < MAX_CONTROLLERS; ++i) {
        if (SDL_IsGameController(i)) {
            openController(i);
        }
    }
}

void InputHandler::shutdown() {
    for (int s = 0; s < MAX_CONTROLLERS; ++s) {
        closeControllerSlot(s);
    }
}

void InputHandler::beginFrame() {
    for (auto& [_, state] : keys) {
        state.pressed  = false;
        state.released = false;
        state.repeated = false;
    }
    for (int s = 0; s < MAX_CONTROLLERS; ++s) {
        for (auto& [_, state] : controllerButtons[s]) {
            state.pressed  = false;
            state.released = false;
        }
    }
}

void InputHandler::processEvent(const SDL_Event& e) {
    switch (e.type) {
        // --- keyboard ---
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            SDL_Keycode key = e.key.keysym.sym;
            auto& state = keys[key];
            if (e.type == SDL_KEYDOWN) {
                if (e.key.repeat) {
                    state.repeated = true;
                    onKey(key, KeyAction::REPEAT);
                } else {
                    state.down    = true;
                    state.pressed = true;
                    onKey(key, KeyAction::PRESS);
                }
            } else {
                state.down     = false;
                state.released = true;
                onKey(key, KeyAction::RELEASE);
            }
            break;
        }

        // --- controller ---
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            int slot = getControllerSlot(e.cbutton.which);
            if (slot < 0) break;

            auto btn = static_cast<SDL_GameControllerButton>(e.cbutton.button);
            auto& state = controllerButtons[slot][btn];

            if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                state.down    = true;
                state.pressed = true;
                onControllerButton(btn, ControllerButtonAction::PRESS, slot);
            } else {
                state.down     = false;
                state.released = true;
                onControllerButton(btn, ControllerButtonAction::RELEASE, slot);
            }
            break;
        }
        case SDL_CONTROLLERAXISMOTION: {
            int slot = getControllerSlot(e.caxis.which);
            if (slot < 0) break;
            axisValues[slot][static_cast<SDL_GameControllerAxis>(e.caxis.axis)] = e.caxis.value;
            break;
        }

        case SDL_CONTROLLERDEVICEADDED:
            if (numControllers() < MAX_CONTROLLERS) {
                openController(e.cdevice.which);
            }
            break;

        case SDL_CONTROLLERDEVICEREMOVED: {
            int slot = getControllerSlot(e.cdevice.which);
            if (slot >= 0) closeControllerSlot(slot);
            break;
        }
    }
}


// --- keyboard ---

bool InputHandler::isDown(SDL_Keycode key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second.down;
}
bool InputHandler::isPressed(SDL_Keycode key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second.pressed;
}
bool InputHandler::isReleased(SDL_Keycode key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second.released;
}
bool InputHandler::isRepeated(SDL_Keycode key) const {
    auto it = keys.find(key);
    return it != keys.end() && it->second.repeated;
}


// --- controller ---

bool InputHandler::isDown(SDL_GameControllerButton button, int ctrl) const {
    if (ctrl < 0 || ctrl >= MAX_CONTROLLERS) return false;
    auto it = controllerButtons[ctrl].find(button);
    return it != controllerButtons[ctrl].end() && it->second.down;
}
bool InputHandler::isPressed(SDL_GameControllerButton button, int ctrl) const {
    if (ctrl < 0 || ctrl >= MAX_CONTROLLERS) return false;
    auto it = controllerButtons[ctrl].find(button);
    return it != controllerButtons[ctrl].end() && it->second.pressed;
}
bool InputHandler::isReleased(SDL_GameControllerButton button, int ctrl) const {
    if (ctrl < 0 || ctrl >= MAX_CONTROLLERS) return false;
    auto it = controllerButtons[ctrl].find(button);
    return it != controllerButtons[ctrl].end() && it->second.released;
}

Sint16 InputHandler::getControllerAxis(SDL_GameControllerAxis axis, int ctrl) const {
    if (ctrl < 0 || ctrl >= MAX_CONTROLLERS) return 0;
    auto it = axisValues[ctrl].find(axis);
    return it != axisValues[ctrl].end() ? it->second : 0;
}

float InputHandler::getNormalizedAxis(SDL_GameControllerAxis axis, int ctrl, int deadzone) const {
    Sint16 raw = getControllerAxis(axis, ctrl);
    bool isTrigger = (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                      axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    float value = isTrigger ? raw / 32767.0f : raw / 32768.0f;

    if (std::abs(value) < deadzone / 32768.0f)
        return 0.0f;

    float sign = (value > 0) ? 1.0f : -1.0f;
    return sign * (std::abs(value) - deadzone / 32768.0f) / (1.0f - deadzone / 32768.0f);
}

int InputHandler::numControllers() const {
    int n = 0;
    for (auto* c : controllers) if (c) ++n;
    return n;
}

int InputHandler::getControllerSlot(SDL_JoystickID instanceId) const {
    for (int s = 0; s < MAX_CONTROLLERS; ++s)
        if (controllers[s] && controllerInstanceIds[s] == instanceId)
            return s;
    return -1;
}


// --- helper methods ---

int InputHandler::findFreeSlot() const {
    for (int s = 0; s < MAX_CONTROLLERS; ++s)
        if (!controllers[s]) return s;
    return -1;
}

void InputHandler::openController(int deviceIndex) {
    int slot = findFreeSlot();
    if (slot < 0) return;

    SDL_GameController* gc = SDL_GameControllerOpen(deviceIndex);
    if (!gc) return;

    controllers[slot]            = gc;
    controllerInstanceIds[slot]  = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
    controllerButtons[slot].clear();
    axisValues[slot].clear();
}

void InputHandler::closeControllerSlot(int slot) {
    if (slot < 0 || slot >= MAX_CONTROLLERS || !controllers[slot]) return;
    SDL_GameControllerClose(controllers[slot]);
    controllers[slot]           = nullptr;
    controllerInstanceIds[slot] = -1;
    controllerButtons[slot].clear();
    axisValues[slot].clear();
}