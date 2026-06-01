#pragma once

#include "ui/Screen.h"

#include <SDL2/SDL_mixer.h>

#include <memory>
#include <vector>


class ScreenStack {
public:
    bool empty() const { return screens.empty(); }
    explicit operator bool() const { return !empty(); }

    Screen* current();
    const Screen* current() const;

    template<typename T>
    T* currentAs() {
        return dynamic_cast<T*>(current());
    }

    void push(std::unique_ptr<Screen> screen, bool playSound = true);
    void replace(std::unique_ptr<Screen> screen, bool playSound = true);
    void pop(bool playSound = true);
    bool goBack(bool playSound = true);
    void clear(bool playSound = false);
    void clearAndPush(std::unique_ptr<Screen> screen, bool playSound = true);
    ScreenAction applyTransition(ScreenTransition transition);

    void handle(const SDL_Event& event);
    void update();

private:
    std::vector<std::unique_ptr<Screen>> screens;

    void playTransitionSound(bool playSound) const;
};
