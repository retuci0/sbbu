#pragma once

#include "../Widget.h"

#include "../../misc/Common.h"
#include "../../misc/Renderer.h"

#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <string>


class Slider : public Widget {
public:
    Slider(int x, int y, int w, int h, float minVal, float maxVal, float initialVal, const std::string& label)
    :  Widget(x, y, w, h), minVal(minVal), maxVal(maxVal), value(initialVal), label(label) {}

    void draw(SDL_Renderer* renderer, TTF_Font* font) override {
        // track bg
        Renderer::fillRect(renderer, rect.x, rect.y, rect.w, rect.h, {100, 100, 100, 255});
    
        // filled part
        int fillW = valueToX(value) - rect.x;
        Renderer::fillRect(renderer, rect.x, rect.y, fillW, rect.h, {100, 180, 100, 255});
    
        // handle
        int hx = valueToX(value);
        int hy = rect.y + rect.h / 2;
        Renderer::fillRect(renderer,
            hx - HANDLE_R, hy - HANDLE_R,
            HANDLE_R * 2,  HANDLE_R * 2,
            {200, 200, 200, 255});
    
        // label with percentage
        int displayPct = static_cast<int>(std::round(value * 100.0f));
        Renderer::renderText(renderer, font,
            label + ": " + std::to_string(displayPct) + "%",
            rect.x, rect.y - 50, WHITE);
    }


    bool handle(const SDL_Event& e) override {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int hy = rect.y + rect.h / 2;
            if (e.button.x >= rect.x && e.button.x <= rect.x + rect.w &&
                std::abs(e.button.y - hy) <= HANDLE_R * 2)
            {
                dragging = true;
                value = xToValue(e.button.x);  // jump to the clicked pos
                return true;
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP && dragging) {
            dragging = false;
            return true;
        }
        if (e.type == SDL_MOUSEMOTION && dragging) {
            value = xToValue(e.motion.x);
            return true;
        }
        return false;
    }

    float getValue() const { return value; }
    void  setValue(float v) { value = std::clamp(v, minVal, maxVal); }

private:
    float minVal, maxVal;
    float value;
    std::string label;
    bool dragging = false;

    static constexpr int HANDLE_R = 18;
    static constexpr int TRACK_H  = 20;

    int valueToX(float v) const {
        float t = (v - minVal) / (maxVal - minVal);
        return rect.x + static_cast<int>(t * rect.w);
    }
    
    float xToValue(int x) const {
        float t = std::clamp((x - rect.x) / static_cast<float>(rect.w), 0.0f, 1.0f);
        return minVal + t * (maxVal - minVal);
    }
};