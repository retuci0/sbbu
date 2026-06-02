#pragma once

#include "ui/Widget.h"
#include "core/Resources.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>
#include <vector>

class Screen;

enum class ScreenAction {
    NONE,
    POP,
    PUSH,
    REPLACE,
    POP_ALL,
    QUIT_GAME
};

struct ScreenTransition {
    ScreenTransition() = default;
    ScreenTransition(ScreenAction action, std::unique_ptr<Screen> next, bool playSound = true);
    ScreenTransition(ScreenTransition&& other) noexcept;
    ScreenTransition& operator=(ScreenTransition&& other) noexcept;
    ~ScreenTransition();

    ScreenTransition(const ScreenTransition&) = delete;
    ScreenTransition& operator=(const ScreenTransition&) = delete;

    ScreenAction action = ScreenAction::NONE;
    std::unique_ptr<Screen> next;
    bool playSound = true;
};


class Screen {
public:
    Screen() {
        titleFont = Resources::get().titleFont;
        font = Resources::get().font;
        smallFont = Resources::get().smallFont;
    }

    virtual ~Screen() {}

    virtual void handle(const SDL_Event& e) {
        for (auto& w : widgets) w->handle(e);
    }

    virtual void update() {}
    virtual void render(SDL_Renderer* r) = 0;
    
    virtual bool isTransparent() const {
        return false;
    }

    bool hasTransition() const {
        return transition.action != ScreenAction::NONE;
    }

    ScreenTransition takeTransition() {
        ScreenTransition pending = std::move(transition);
        transition = {};
        return pending;
    }

protected:
    TTF_Font* titleFont;
    TTF_Font* font;
    TTF_Font* smallFont;

    std::vector<std::unique_ptr<Widget>> widgets;

    template<typename T, typename... Args>
    T* addWidget(Args&&... args) {
        auto w = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = w.get();
        widgets.push_back(std::move(w));
        return ptr;
    }

    void drawWidgets(SDL_Renderer* renderer, TTF_Font* font) {
        for (auto& w : widgets) w->draw(renderer, font);
    }

    void requestTransition(ScreenAction action, std::unique_ptr<Screen> next = nullptr, bool playSound = true) {
        transition = ScreenTransition(action, std::move(next), playSound);
    }

    void goBack(bool playSound = true) {
        requestTransition(ScreenAction::POP, nullptr, playSound);
    }

    void pushScreen(std::unique_ptr<Screen> screen, bool playSound = true) {
        requestTransition(ScreenAction::PUSH, std::move(screen), playSound);
    }

    void replaceWith(std::unique_ptr<Screen> screen, bool playSound = true) {
        requestTransition(ScreenAction::REPLACE, std::move(screen), playSound);
    }

    void popAll(bool playSound = false) {
        requestTransition(ScreenAction::POP_ALL, nullptr, playSound);
    }

    void quitGame() {
        requestTransition(ScreenAction::QUIT_GAME);
    }

private:
    ScreenTransition transition;
};

inline ScreenTransition::ScreenTransition(ScreenAction action, std::unique_ptr<Screen> next, bool playSound)
    : action(action), next(std::move(next)), playSound(playSound) {}

inline ScreenTransition::ScreenTransition(ScreenTransition&& other) noexcept = default;
inline ScreenTransition& ScreenTransition::operator=(ScreenTransition&& other) noexcept = default;
inline ScreenTransition::~ScreenTransition() = default;
