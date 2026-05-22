#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <vector>


struct CharacterStats {
    std::string name;
    std::string description;
    int   health           = 100;
    int   damage           = 5;
    int   projectileDamage = 3;
    float velocity         = 5.0f;
    float jumpVelocity     = 11.0f;
    float critChance       = 0.2f;
    float gravity          = 0.5f;
    float terminalVelocity = 20.0f;
    float weight           = 1.0f;  // lower = less knockback
};

struct Character {
    CharacterStats stats;

    // only right-facing frames are stored
    std::vector<SDL_Texture*> walkFrames;   // 5 frames
    std::vector<SDL_Texture*> jumpFrames;   // 5 frames
    std::vector<SDL_Texture*> attackFrames; // 5 frames
    SDL_Texture* idle       = nullptr;
    SDL_Texture* shoot      = nullptr;
    SDL_Texture* damage     = nullptr;
    SDL_Texture* icon       = nullptr;
    SDL_Texture* deadIcon   = nullptr;

    bool loaded = false;
    void unload();
};

extern const CharacterStats BERT_STATS;
extern const CharacterStats BERROTA_STATS;
extern const CharacterStats LORC_STATS;
extern const CharacterStats JORDI_STATS;
extern const CharacterStats BARCOS_STATS;
extern const CharacterStats ALSEXITO_STATS;

Character loadCharacter(SDL_Renderer* renderer, const CharacterStats& stats, const std::string& assetFolder);