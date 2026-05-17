#pragma once

#include "../misc/characters.h"
#include "../misc/common.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <string>
#include <vector>

class Platform;

enum class Status { 
    IDLE, 
    WALKING, 
    JUMPING, 
    ATTACKING, 
    DAMAGED 
};


class Player {
public:
    std::string name;
    const Character* character = nullptr;  // non-owning

    float dx = 0.0f, dy = 0.0f;

    // state
    Status status = Status::IDLE;
    Facing facing = Facing::RIGHT;
    int hp        = 0;
    int lives     = 2;  // remaining (total = 3)

    bool onGround     = false;
    bool hasAirJumped = false;

    // hitbox
    SDL_Rect rect = {};

    // animation
    float currentSpriteIndex = 0.f;

    // sound (non-owning pointers)
    Mix_Chunk* damage_sound = nullptr;

    Player() = default;
    void init(int x, int y, const Character* ch, const std::string& playerName, Mix_Chunk* dmgSound);

    void move(int direction);  // -1 left, 0 stop, +1 right
    void jump();
    void getHit(Facing side);

    void update(const std::vector<Platform>& platforms);
    void draw(SDL_Renderer* r) const;
    void drawHitboxes(SDL_Renderer* r) const;

private:
    void animate();
    void drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH) const;
};