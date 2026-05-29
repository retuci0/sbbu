#pragma once

#include "misc/Common.h"
#include "obj/Player.h"

#include <SDL2/SDL.h>


class Projectile {
public:
    SDL_Rect rect = {};
    SDL_Rect prevRect = {};
    Facing direction;
    float velocity = 19.0f;
    float parryFreezeTimer = 0.0f;
    float parryFlashTimer = 0.0f;
    Player* owner = nullptr;  // non-owning

    SDL_Texture* img = nullptr;  // non-owning

    Projectile(int x, int y, Facing dir, Player* owner);

    static constexpr int PARRY_FREEZE_DURATION = 12;
    static constexpr int PARRY_FLASH_DURATION = 10;

    void update(float ts);
    void parry(Player* newOwner);
    void draw(SDL_Renderer* r, float a);
    void drawHitbox(SDL_Renderer* r, float a) const;
};
