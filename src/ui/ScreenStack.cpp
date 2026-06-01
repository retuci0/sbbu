#include "ui/ScreenStack.h"

#include "core/Resources.h"


Screen* ScreenStack::current() {
    return screens.empty() ? nullptr : screens.back().get();
}

const Screen* ScreenStack::current() const {
    return screens.empty() ? nullptr : screens.back().get();
}

void ScreenStack::push(std::unique_ptr<Screen> screen, bool playSound) {
    if (!screen) return;
    screens.push_back(std::move(screen));
    playTransitionSound(playSound);
}

void ScreenStack::replace(std::unique_ptr<Screen> screen, bool playSound) {
    if (screens.empty()) {
        push(std::move(screen), playSound);
        return;
    }

    screens.back() = std::move(screen);
    if (!screens.back()) {
        screens.pop_back();
    }
    playTransitionSound(playSound);
}

void ScreenStack::pop(bool playSound) {
    if (screens.empty()) return;
    screens.pop_back();
    playTransitionSound(playSound);
}

bool ScreenStack::goBack(bool playSound) {
    if (screens.empty()) return false;
    pop(playSound);
    return true;
}

void ScreenStack::clear(bool playSound) {
    if (screens.empty()) return;
    screens.clear();
    playTransitionSound(playSound);
}

void ScreenStack::clearAndPush(std::unique_ptr<Screen> screen, bool playSound) {
    screens.clear();
    push(std::move(screen), playSound);
}

ScreenAction ScreenStack::applyTransition(ScreenTransition transition) {
    switch (transition.action) {
        case ScreenAction::NONE:
            break;
        case ScreenAction::POP:
            goBack(transition.playSound);
            break;
        case ScreenAction::PUSH:
            push(std::move(transition.next), transition.playSound);
            break;
        case ScreenAction::REPLACE:
            replace(std::move(transition.next), transition.playSound);
            break;
        case ScreenAction::POP_ALL:
            if (transition.next) {
                clearAndPush(std::move(transition.next), transition.playSound);
            } else {
                clear(transition.playSound);
            }
            break;
        case ScreenAction::QUIT_GAME:
            return ScreenAction::QUIT_GAME;
    }

    return transition.action;
}

void ScreenStack::handle(const SDL_Event& event) {
    if (Screen* screen = current()) {
        screen->handle(event);
    }
}

void ScreenStack::update() {
    if (Screen* screen = current()) {
        screen->update();
    }
}

void ScreenStack::playTransitionSound(bool playSound) const {
    if (playSound) Mix_PlayChannel(-1, Resources::get().getSound("select"), 0);
}
