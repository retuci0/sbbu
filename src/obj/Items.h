#pragma once

#include "core/Resources.h"
#include "obj/Entity.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <string>
#include <vector>


class Player;
class Platform;
class Projectile;

constexpr float SHIT_ITEM_DURATION      = 600.0f;
constexpr float MUSHROOM_ITEM_DURATION  = 600.0f;
constexpr float COCAINE_ITEM_DURATION   = 500.0f;
constexpr float SPRING_ITEM_DURATION    = 500.0f;

class Item : public Entity {
public:
    Item(const std::string& name, SDL_Rect spawnRect, SDL_Texture* tex, Mix_Chunk* sfx,
         float effectDuration = 0.0f, float respawnDelay = 0.0f);
    virtual ~Item() = default;

    virtual void onPickup();
    virtual void onEffectEnd();
    
    virtual void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
    virtual void draw(SDL_Renderer* r, float a) override;
    virtual void drawEffect(SDL_Renderer* r, float a) const {}
    virtual void drawHitbox(SDL_Renderer* r, float a) const override;

    void takeDamage(int damage, Facing side, float kbScale = 1.0f);
    void kill() { hp = 0; active = false; }
    bool isAlive()  const { return hp > 0; }
    bool isActive() const { return active; }
    bool isAffecting() const { return effectTimer > 0.0f; }

    Player*  consumer = nullptr;
    
    float effectTimer = 0.0f;

protected:
    bool     active         = true;
    float    effectDuration = 0.0f;
    float    respawnTimer   = 0.0f;
    float    respawnDelay   = 0.0f;

    int      hp             = 30;
    bool     onGround       = false;
    
    Mix_Chunk*   sfx        = nullptr;

private:
    std::string  name;
    SDL_Rect     spawnRect  = {};

    static constexpr float GRAVITY      = 0.5f;
    static constexpr float TERMINAL_VEL = 20.0f;
    static constexpr float KB_FRICTION  = 0.85f;
};


/////////////////////////////////////////
/*                ITEMS                */
/////////////////////////////////////////

class MushroomItem : public Item {
public:
    MushroomItem(int x, int y)
    : Item("mushroom",
           { x, y, 56, 56 },
           Resources::get().getTexture("item_mushroom"),
           Resources::get().getSound("item_mushroom"),
           MUSHROOM_ITEM_DURATION,
           0.0f)
    {}

    void onPickup()    override;
    void onEffectEnd() override;

private:
    void scalePlayer(Player* player, float k);
};

class ShitItem : public Item {
public:
    ShitItem(int x, int y)
    : Item("shit",
           { x, y, 56, 56 },
           Resources::get().getTexture("item_shit"),
           Resources::get().getSound("item_shit"),
           SHIT_ITEM_DURATION,
           0.0f)
    {}

    void onPickup()    override;
    void onEffectEnd() override;
    void drawEffect(SDL_Renderer* r, float a) const override;
};

class CocaineItem : public Item {
public:
    CocaineItem(int x, int y)
    : Item("cocaine",
    { x, y, 56, 56 },
    Resources::get().getTexture("item_cocaine"),
    Resources::get().getSound("item_cocaine"),
    COCAINE_ITEM_DURATION,
    0.0f)
    , overlay(Resources::get().getTexture("high_overlay"))
    {}

    void onPickup()    override;
    void onEffectEnd() override;
    void drawEffect(SDL_Renderer* r, float a) const override;

private:
    SDL_Texture* overlay = nullptr;
};

class SpringItem : public Item {
public:
    SpringItem(int x, int y)
    : Item("spring",
    { x, y, 56, 56 },
    Resources::get().getTexture("item_spring"),
    Resources::get().getSound("item_spring"),
    SPRING_ITEM_DURATION,
    0.0f)
    , overlay(Resources::get().getTexture("spring_overlay"))
    {}

    void onPickup()    override;
    void onEffectEnd() override;
    void drawEffect(SDL_Renderer* r, float a) const override;

private:
    SDL_Texture* overlay = nullptr;
};