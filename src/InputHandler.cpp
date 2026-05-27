#include "InputHandler.h"
#include <SDL2/SDL.h>


void InputHandler::init() {
    SDL_GameControllerEventState(SDL_ENABLE);
    if (SDL_NumJoysticks() > 0 && SDL_IsGameController(0)) {
        openController(0);
    }
}

void InputHandler::shutdown() {
    closeController();
}

void InputHandler::beginFrame() {
    for (auto& [_, state] : keys) {
        state.pressed = false;
        state.released = false;
        state.repeated = false;
    }
    for (auto& [_, state] : controllerButtons) {
        state.pressed = false;
        state.released = false;
    }
}

void InputHandler::processEvent(const SDL_Event& e) {
    switch (e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            SDL_Keycode key = e.key.keysym.sym;
            auto& state = keys[key];
            if (e.type == SDL_KEYDOWN) {
                if (e.key.repeat) {
                    state.repeated = true;
                    onKey(key, KeyAction::REPEAT);
                } else {
                    state.down = true;
                    state.pressed = true;
                    onKey(key, KeyAction::PRESS);
                }
            } else {
                state.down = false;
                state.released = true;
                onKey(key, KeyAction::RELEASE);
            }
            break;
        }

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            if (!controller) break;
            SDL_GameControllerButton btn = static_cast<SDL_GameControllerButton>(e.cbutton.button);
            auto& state = controllerButtons[btn];

            if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                state.down = true;
                state.pressed = true;
                onControllerButton(btn, ControllerButtonAction::PRESS);

                // Map controller buttons to keyboard keys
                SDL_Keycode mappedKey = 0;
                switch (btn) {
                    case SDL_CONTROLLER_BUTTON_A:          mappedKey = SDLK_RETURN; break;
                    case SDL_CONTROLLER_BUTTON_B:          mappedKey = SDLK_ESCAPE; break;
                    case SDL_CONTROLLER_BUTTON_X:          mappedKey = SDLK_TAB;    break;
                    case SDL_CONTROLLER_BUTTON_Y:          mappedKey = SDLK_BACKSPACE; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:    mappedKey = SDLK_UP;      break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  mappedKey = SDLK_DOWN;    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  mappedKey = SDLK_LEFT;    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: mappedKey = SDLK_RIGHT;   break;
                    case SDL_CONTROLLER_BUTTON_START:      mappedKey = SDLK_RETURN;  break;
                    case SDL_CONTROLLER_BUTTON_BACK:       mappedKey = SDLK_ESCAPE;  break;
                    default: break;
                }
                if (mappedKey) {
                    auto& keyState = keys[mappedKey];
                    keyState.down = true;
                    keyState.pressed = true;
                    onKey(mappedKey, KeyAction::PRESS);
                }
            } else {
                state.down = false;
                state.released = true;
                onControllerButton(btn, ControllerButtonAction::RELEASE);

                SDL_Keycode mappedKey = 0;
                switch (btn) {
                    case SDL_CONTROLLER_BUTTON_A:          mappedKey = SDLK_RETURN; break;
                    case SDL_CONTROLLER_BUTTON_B:          mappedKey = SDLK_ESCAPE; break;
                    case SDL_CONTROLLER_BUTTON_X:          mappedKey = SDLK_TAB;    break;
                    case SDL_CONTROLLER_BUTTON_Y:          mappedKey = SDLK_BACKSPACE; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:    mappedKey = SDLK_UP;      break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  mappedKey = SDLK_DOWN;    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  mappedKey = SDLK_LEFT;    break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: mappedKey = SDLK_RIGHT;   break;
                    case SDL_CONTROLLER_BUTTON_START:      mappedKey = SDLK_RETURN;  break;
                    case SDL_CONTROLLER_BUTTON_BACK:       mappedKey = SDLK_ESCAPE;  break;
                    default: break;
                }
                if (mappedKey) {
                    auto& keyState = keys[mappedKey];
                    keyState.down = false;
                    keyState.released = true;
                    onKey(mappedKey, KeyAction::RELEASE);
                }
            }
            break;
        }

        case SDL_CONTROLLERAXISMOTION: {
            if (!controller) break;
            axisValues[static_cast<SDL_GameControllerAxis>(e.caxis.axis)] = e.caxis.value;
            break;
        }

        case SDL_CONTROLLERDEVICEADDED:
            if (!controller) openController(e.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            if (controller && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) == e.cdevice.which) {
                closeController();
                if (SDL_NumJoysticks() > 0 && SDL_IsGameController(0))
                    openController(0);
            }
            break;
    }
}
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

bool InputHandler::isDown(SDL_GameControllerButton button) const {
    auto it = controllerButtons.find(button);
    return it != controllerButtons.end() && it->second.down;
}
bool InputHandler::isPressed(SDL_GameControllerButton button) const {
    auto it = controllerButtons.find(button);
    return it != controllerButtons.end() && it->second.pressed;
}
bool InputHandler::isReleased(SDL_GameControllerButton button) const {
    auto it = controllerButtons.find(button);
    return it != controllerButtons.end() && it->second.released;
}

Sint16 InputHandler::getControllerAxis(SDL_GameControllerAxis axis) const {
    auto it = axisValues.find(axis);
    return it != axisValues.end() ? it->second : 0;
}

float InputHandler::getNormalizedAxis(SDL_GameControllerAxis axis, int deadzone) const {
    Sint16 raw = getControllerAxis(axis);
    bool isTrigger = (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    float value = isTrigger ? raw / 32767.0f : raw / 32768.0f;

    if (std::abs(value) < deadzone / 32768.0f)
        return 0.0f;

    float sign = (value > 0) ? 1.0f : -1.0f;
    return sign * (std::abs(value) - deadzone / 32768.0f) / (1.0f - deadzone / 32768.0f);
}

void InputHandler::openController(int deviceIndex) {
    closeController();
    controller = SDL_GameControllerOpen(deviceIndex);
    if (controller) {
        controllerButtons.clear();
        axisValues.clear();
    }
}

void InputHandler::closeController() {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
        controllerButtons.clear();
        axisValues.clear();
    }
}