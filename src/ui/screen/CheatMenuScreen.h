#pragma once

#include "ui/Screen.h"
#include "ui/widget/FieldWidget.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <optional>


class Game;

class CheatMenuScreen : public Screen {
public:
    CheatMenuScreen(Game* game);

    void handle(const SDL_Event& event) override;
    void render(SDL_Renderer* renderer) override;

    bool isTransparent() const override { return true; }

private:
    FieldWidget* xField  = nullptr;
    FieldWidget* yField  = nullptr;
    FieldWidget* hpField = nullptr;

    int selectedIndex = 0;
    std::string error;

    std::optional<int> getX() {
        try { return std::stoi(xField->getText()); }
        catch (const std::invalid_argument&) { error = "invalid x coord!"; return std::nullopt; }
    }

    std::optional<int> getY() {
        try { return std::stoi(yField->getText()); }
        catch (const std::invalid_argument&) { error = "invalid y coord!"; return std::nullopt; }
    }

    std::optional<int> getHp() {
        try {
            int v = std::stoi(hpField->getText());
            if (v < 0) { error = "hp can't be negative!"; return std::nullopt; }
            return v;
        } catch (const std::invalid_argument&) { error = "invalid hp value!"; return std::nullopt; }
    }
};