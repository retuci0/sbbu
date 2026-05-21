#pragma once

#include "../misc/Characters.h"
#include "../misc/Common.h"
#include "../misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <uuid/uuid.h>
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

    uuid_t uuid;

    float dx = 0.0f, dy = 0.0f;

    // state
    Status status = Status::IDLE;
    Facing facing = Facing::RIGHT;
    int hp        = 0;
    int lives     = 2;  // remaining (total = 3)

    bool onGround     = false;
    bool hasAirJumped = false;

    static constexpr int EDGE_CLIMB_THRESHOLD   = 16;

    int damagedTimer                            = 0;
    static constexpr int DAMAGED_DURATION       = 10;
    int shootCooldown                           = 0;
    static constexpr int SHOOT_COOLDOWN         = 25;
    int meleeTimer                              = 0;
    static constexpr int MELEE_DURATION         = 8; 
    int meleeCooldown                           = 0;
    static constexpr int MELEE_COOLDOWN         = 20;
    int invulnerableTimer                       = 0;
    static constexpr int INV_DURATION           = 300;

    float charge = 0.0f;
    static constexpr float MAX_CHARGE = 1.0f;

    // hitbox
    SDL_Rect rect = {};

    // animation
    float currentSpriteIndex = 0.0f;

    // player color indicator (drawn above head)
    Color color = { 255, 255, 255, 255 };

    // non-owning
    Mix_Chunk* damageSound = nullptr;

    // sprite in use
    SDL_Texture* currentTexture = nullptr;

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

    void draw(SDL_Renderer* r, TTF_Font* font);
    void drawNametag(SDL_Renderer* r, TTF_Font* font) const;
    void drawHitbox(SDL_Renderer* r) const;
    std::string getStatusName() const;

    bool operator==(const Player& other) const {
        return uuid_compare(uuid, other.uuid) == 0;
    }

private:
    void animate();
    void drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH);
};