#pragma once

#include "core/Resources.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <string>
#include <vector>


class Player;
class Platform;
class Projectile;

constexpr int SHIT_AURA_DURATION = 600.0f;

class Item {
public:
    Item(const std::string& name, SDL_Rect spawnRect, SDL_Texture* tex, Mix_Chunk* sfx,
         float effectDuration = 0.0f, float respawnDelay = 0.0f);
    virtual ~Item() = default;

    virtual void onPickup();
    virtual void onEffectEnd() {}
    virtual void update(std::vector<Player*>& players,
                        const std::vector<Platform>& platforms,
                        const std::vector<Projectile>& projectiles,
                        float ts);
    virtual void draw(SDL_Renderer* r, float a) const;
    virtual void drawHitbox(SDL_Renderer* r, float a) const;

    void takeDamage(int damage, Facing side, float kbScale = 1.0f);
    bool isAlive()  const { return hp > 0; }
    bool isActive() const { return active; }

    SDL_Rect rect     = {};
    SDL_Rect prevRect = {};

    Player*  consumer = nullptr;
    
    float effectTimer = 0.0f;

protected:
    bool     active         = true;
    float    effectDuration = 0.0f;
    float    respawnTimer   = 0.0f;
    float    respawnDelay   = 0.0f;

    int      hp             = 30;
    float    dx             = 0.0f;
    bool     onGround       = false;

private:
    std::string  name;
    SDL_Rect     spawnRect  = {};
    SDL_Texture* tex        = nullptr;
    Mix_Chunk*   sfx        = nullptr;
    float        dy         = 0.0f;

    static constexpr float GRAVITY      = 0.5f;
    static constexpr float TERMINAL_VEL = 20.0f;
    static constexpr float KB_FRICTION  = 0.85f;
};


class MushroomItem : public Item {
public:
    MushroomItem(int x, int y)
    : Item("mushroom",
           { x, y, 56, 56 },
           Resources::get().getTexture("item_mushroom"),
           Resources::get().getSound("item_mushroom"),
           600.0f,
           0.0f)
    {}

    void onPickup()    override;
    void onEffectEnd() override;

private:
    int  prevDmg = 0;
    int  prevProjDmg = 0;
    void scalePlayer(Player* player, float k);
};

class ShitItem : public Item {
public:
    ShitItem(int x, int y)
    : Item("shit",
           { x, y, 56, 56 },
           Resources::get().getTexture("item_shit"),
           Resources::get().getSound("item_shit"),
           SHIT_AURA_DURATION,
           0.0f)
    {}

    void onPickup()    override;
    void onEffectEnd() override;
};