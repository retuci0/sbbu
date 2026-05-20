#pragma once

#include "../misc/Characters.h"
#include "../misc/Common.h"
#include "../misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>


class Platform;
class Projectile;

enum class Status {
    IDLE,
    WALKING,
    JUMPING,
    SHOOTING,
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

    int damagedTimer    = 0;
    int shootCooldown   = 0;
    int meleeTimer      = 0;
    int meleeCooldown   = 0;

    // hitbox
    SDL_Rect rect = {};

    // animation
    float currentSpriteIndex = 0.0f;

    // player color indicator (drawn above head)
    Color color = { 255, 255, 255, 255 };

    // non-owning
    Mix_Chunk* damageSound = nullptr;

    Player() = default;
    void init(int x, int y, const Character* ch, const std::string& playerName, Mix_Chunk* dmgSound);

    void move(int direction);  // -1 left, 0 stop, +1 right
    void jump();

    void getHit(Facing side);
    bool tryShoot(Mix_Chunk* projSound);
    bool tryMelee(Mix_Chunk* meleeSound);

    void update(const std::vector<Platform>& platforms);
    void updateTimers();
    void resetTimers();

    void draw(SDL_Renderer* r, TTF_Font* font) const;
    void drawNametag(SDL_Renderer* r, TTF_Font* font) const;
    void drawHitbox(SDL_Renderer* r) const;
    std::string getStatusName() const;

private:
    void animate();
    void drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH) const;
};