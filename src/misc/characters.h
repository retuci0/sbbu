#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <vector>


struct CharacterStats {
    std::string name;
    std::string description;
    int   health            = 100;
    int   damage            = 5;
    int   projectile_damage = 3;
    float velocity          = 5.0f;
    float jump_velocity     = 11.0f;
    float crit_chance       = 0.2f;
    float gravity           = 0.5f;
    float terminal_velocity = 20.0f;
    float weight            = 1.0f;  // lower = less knockback
};

struct Character {
    CharacterStats stats;

    // only right-facing frames are stored
    std::vector<SDL_Texture*> walk_frames;  // 5 frames
    std::vector<SDL_Texture*> jump_frames;  // 5 frames
    SDL_Texture* idle   = nullptr;
    SDL_Texture* attack = nullptr;
    SDL_Texture* damage = nullptr;
    SDL_Texture* icon   = nullptr;

    bool loaded = false;
    void unload();
};

extern const CharacterStats BERT_STATS;
extern const CharacterStats BERROTA_STATS;
extern const CharacterStats LORC_STATS;
extern const CharacterStats JORDI_STATS;

Character loadCharacter(SDL_Renderer* renderer, const CharacterStats& stats, const std::string& assetFolder);