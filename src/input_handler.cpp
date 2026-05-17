#include "input_handler.h"


void InputHandler::beginFrame() {
    for (auto& [_, state] : keys) {
        state.pressed = false;
        state.released = false;
        state.repeated = false;
    }
}

void InputHandler::processEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN && e.type != SDL_KEYUP)
        return;

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
    }
    else if (e.type == SDL_KEYUP) {
        state.down = false;
        state.released = true;
        onKey(key, KeyAction::RELEASE);
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