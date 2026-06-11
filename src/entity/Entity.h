#pragma once

#include "misc/Common.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <memory>
#include <vector>


enum class EntityType {
    PLAYER,    // red hitboxes
    PLATFORM,  // blue hitboxes
    MISC       // green hitboxes
};

class Entity {
public:
    Entity(SDL_Rect rect = {}, float dx = 0.0f, float dy = 0.0f, SDL_Texture* tex = nullptr)
        : rect(rect), prevRect(rect)
        , dx(dx), dy(dy)
        , tex(tex) {}

    virtual ~Entity() = default;

    virtual void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
        prevRect = rect;
        rect.x += dx * ts;
        rect.y += dy * ts;
    }

    virtual void draw(SDL_Renderer* renderer, float a) {}
    virtual void drawHitbox(SDL_Renderer* renderer, float a) const {}

    SDL_Rect rect = {};
    // for interpolation
    SDL_Rect prevRect = {};

    Facing facing = Facing::RIGHT;

    SDL_Texture* tex = nullptr;

    // velocity
    float dx = 0.0f, dy = 0.0f;

    virtual bool intersectsWith(const Entity& other) const {
        return SDL_HasIntersection(&rect, &other.rect);
    }

    SDL_Rect interpolatedRect(float alpha) const {
        if (PauseManager::paused) return rect;
        return {
            static_cast<int>(prevRect.x + (rect.x - prevRect.x) * alpha),
            static_cast<int>(prevRect.y + (rect.y - prevRect.y) * alpha),
            rect.w,
            rect.h
        };
    }

    static SDL_Rect interpolatedRect(SDL_Rect prevRect, SDL_Rect rect, float alpha) {
        if (PauseManager::paused || alpha < 0) return rect;
        return {
            static_cast<int>(prevRect.x + (rect.x - prevRect.x) * alpha),
            static_cast<int>(prevRect.y + (rect.y - prevRect.y) * alpha),
            rect.w,
            rect.h
        };
    }

    virtual EntityType getType() const {
        return EntityType::MISC;
    }
};
