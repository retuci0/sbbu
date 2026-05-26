#pragma once

#include "../misc/Common.h"
#include "../objects/Player.h"

#include <SDL2/SDL.h>


class Projectile {
public:
    SDL_Rect rect = {};
    Facing direction;
    float velocity = 19.0f;
    int parryFreezeTimer = 0;
    int parryFlashTimer = 0;
    Player* owner = nullptr;  // non-owning

    SDL_Texture* img = nullptr;  // non-owning

    Projectile(SDL_Texture* img, int x, int y, Facing dir, Player* owner);

    static constexpr int PARRY_FREEZE_DURATION = 12;
    static constexpr int PARRY_FLASH_DURATION = 10;

    void move();
    void parry(Player* newOwner);
    void draw(SDL_Renderer* r);
    void drawHitbox(SDL_Renderer* r) const;
};
