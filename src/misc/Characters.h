#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

#include <string>
#include <vector>


class Player;

struct CharacterStats {
    std::string name;
    std::string description;
    int   health           = 100;
    int   damage           = 5;
    int   projectileDamage = 3;
    float velocity         = 5.0f;
    float jumpVelocity     = 12.2f;
    float critChance       = 0.2f;
    float gravity          = 0.5f;
    float terminalVelocity = 20.0f;
    float weight           = 1.0f;  // lower = less knockback
};

struct SpecialHitboxParams {
    int   x = 0, y = 0, w = 0, h = 0;
    float damageScale    = 3.0f;
    float kbScale        = 5.0f;
    bool  spawnShockwave = false;
};

class Character {
public:
    CharacterStats stats;

    // only right-facing frames are stored
    std::vector<SDL_Texture*> walkFrames;           // 5 frames
    std::vector<SDL_Texture*> jumpFrames;           // 5 frames
    std::vector<SDL_Texture*> attackFrames;         // 5 frames
    std::vector<SDL_Texture*> stunnedFrames;        // 3 frames
    std::vector<SDL_Texture*> specialStaticFrames;  // 6 frames
    std::vector<SDL_Texture*> specialSideFrames;    // 5 frames
    std::vector<SDL_Texture*> specialUpFrames;      // 5 frames
    std::vector<SDL_Texture*> specialDownFrames;    // 3 frames

    SDL_Texture* idle     = nullptr;
    SDL_Texture* shoot    = nullptr;
    SDL_Texture* damage   = nullptr;
    SDL_Texture* shielded = nullptr;
    SDL_Texture* icon     = nullptr;
    SDL_Texture* deadIcon = nullptr;

    bool loaded = false;
    void unload();

    // override in subclasses for unique specials, defaults are generic
    virtual SpecialHitboxParams specialStatic(Player& player) const;
    virtual SpecialHitboxParams specialSide  (Player& player) const;
    virtual SpecialHitboxParams specialUp    (Player& player) const;
    virtual SpecialHitboxParams specialDown  (Player& player) const;

    // override to apply movement on special activation (called from trySpecial)
    virtual void onSpecialStatic(Player& player) const;
    virtual void onSpecialSide  (Player& player) const;
    virtual void onSpecialUp    (Player& player) const;
    virtual void onSpecialDown  (Player& player) const;

    virtual ~Character() = default;
};

class BertCharacter : public Character { 
public: 
    BertCharacter(); 
};

class BerrotaCharacter : public Character { 
public: 
    BerrotaCharacter(); 
};

class LorcCharacter : public Character {
public:
    LorcCharacter();
    SpecialHitboxParams specialDown(Player& player) const override;
    void onSpecialDown(Player& player) const override;
};

class JordiCharacter : public Character { 
    public: JordiCharacter(); 
};

class BarcosCharacter   : public Character { 
public: 
    BarcosCharacter(); 
};

class AlsexitoCharacter : public Character { 
public: 
    AlsexitoCharacter(); 
};

class FlanCharacter     : public Character { 
public: 
    FlanCharacter(); 
};

void loadCharacter(Character& character, SDL_Renderer* renderer, const std::string& folder);