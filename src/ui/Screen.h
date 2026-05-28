#pragma once

#include "Widget.h"
#include "../Resources.h"

#include <SDL2/SDL.h>

#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>


class Screen {
public:
    Screen() {
        titleFont = Resources::get().titleFont;
        font = Resources::get().font;
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

protected:
    TTF_Font* titleFont;
    TTF_Font* font;

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
};
