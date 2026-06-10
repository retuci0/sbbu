#pragma once

#include "entity/Entity.h"
#include "entity/Items.h"

#include "misc/Characters.h"
#include "misc/Common.h"
#include "misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>


class Platform;
class Projectile;
class Grapple;

enum class Status {
    IDLE,
    WALKING,
    JUMPING,
    SHOOTING,
    ATTACKING,
    DAMAGED,
    SPECIAL_STATIC,
    SPECIAL_SIDE,
    SPECIAL_UP,
    SPECIAL_DOWN,
    SHIELDED,
    STUNNED,
    DASHING
};

class Player : public Entity {
public:
    Player() = default;
    void init(int x, int y, const Character ch, const std::string& playerName);

    std::string name;
    Character character; 

    int id = 0;

    // state
    Status status = Status::IDLE;
    int hp        = 0;
    int lives     = 2;  // remaining (total = 3)

    bool onGround             = false;
    bool hasAirJumped         = false;
    bool specialHitboxSpawned = false;

    static constexpr int EDGE_CLIMB_THRESHOLD = 16;

    float damagedTimer                    = 0.0f;
    static constexpr int DAMAGED_DURATION = 10;
    float shootTimer                      = 0.0f;
    static constexpr int SHOOT_DURATION   = 10;
    float shootCooldown                   = 0.0f;
    static constexpr int SHOOT_COOLDOWN   = 25;
    float meleeTimer                      = 0.0f;
    static constexpr int MELEE_DURATION   = 8;
    float meleeCooldown                   = 0.0f;
    static constexpr int MELEE_COOLDOWN   = 20;
    float invulnerableTimer               = 0.0f;
    static constexpr int INV_DURATION     = 300;
    float specialCooldown                 = 0.0f;
    static constexpr int SPECIAL_COOLDOWN = 60;
    float specialTimer                    = 0.0f;
    static constexpr int SPECIAL_DURATION = 24;
    static constexpr int SPECIAL_HITBOX_DURATION = 5;
    float droppingTimer                   = 0.0f;
    static constexpr int DROP_DURATION    = 15;
    float shitAuraTimer                   = 0.0f;

    float charge = 0.0f;
    static constexpr float MAX_CHARGE = 1.0f;

    float scale = 1.0f;


    // shield

    void setShieldHeld(bool held);
    bool tryShield();
    void releaseShield();
    void breakShield();
    void blockHit(int damage, float kbScale);
    float getShieldScale() const;

    static constexpr int   SHIELD_HP_MAX        = 80;
    static constexpr int   SHIELD_DURATION      = 180;
    static constexpr float SHIELD_HP_DRAIN      = 0.12f;
    static constexpr float SHIELD_HP_REGEN      = 0.25f;
    static constexpr float SHIELD_HP_HIT_COST   = 8.0f;
    static constexpr int   SHIELD_BREAK_STUN    = 180;
    static constexpr int   SHIELD_STUN_DURATION = 14;
    static constexpr float SHIELD_MIN_SIZE      = 0.35f;

    float shieldTimer      = 0.0f;
    float shieldHp         = SHIELD_HP_MAX;
    bool  shieldBroken     = false;
    float shieldBreakTimer = 0.0f;
    float shieldStunTimer  = 0.0f;
    float stunTimer        = 0.0f;
    bool  shieldHeld       = false;

    // grapple
    Grapple* grapple = nullptr;
    void throwGrapple();
    void ungrapple();

    // animation
    float currentSpriteIndex = 0.0f;

    // player color indicator (drawn above head)
    Color color = WHITE;

    // item
    Item* item;
    bool dontResize = false;

    // movement
    void move(int direction);  // -1 left, 0 stop, +1 right
    void jump();
    void dash();
    float dashTimer         = 0.0f;
    static constexpr int DASH_DURATION = 10;
    float dashCooldown      = 0.0f;
    static constexpr int DASH_COOLDOWN = 60;
    float postGrappleTimer  = 0.0f;
    static constexpr int POST_GRAPPLE_DURATION = 30;

    void getHit(Player* attacker, Facing side, int damage, float kbScale = 1.0f);
    bool tryShoot();
    bool tryMelee();
    bool trySpecial(Direction dir);

    void update(std::vector<std::unique_ptr<Entity>>& entities,
                float ts) override;
    void updateTimers(float ts);
    void resetTimers();

    bool downKeyPressed = false;
    void setDownKeyPressed(bool pressed) {
        downKeyPressed = pressed;
    }

    void draw(SDL_Renderer* r, float a) override;
    void drawShield(SDL_Renderer* r, float a) const;
    void drawNametag(SDL_Renderer* r, TTF_Font* font, float a) const;
    void drawShitAura(SDL_Renderer* r, float a) const;
    void drawHitbox(SDL_Renderer* r, float a) const override;
    void animate(float ts);

    std::string getStatusName() const;

    bool operator==(const Player& other) const {
        return id == other.id;
    }

    EntityType getType() const override {
        return EntityType::PLAYER;
    }

private:
    void drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH, float a);
};