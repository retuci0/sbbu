#include "Characters.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_log.h>


const CharacterStats BERT_STATS = {
    "Bert", "average human.",
    100, 5, 3, 5.0f, 11.0f, 0.20f, 0.50f, 20.0f, 1.0f
};
const CharacterStats BERROTA_STATS = {
    "Berrota", "lightweight skinny mf, moves faster than your mom.",
    90, 5, 3, 7.0f, 13.0f, 0.35f, 0.50f, 17.0f, 1.5f
};
const CharacterStats LORC_STATS = {
    "Lorc", "heavy tank - only weakness is gravity.",
    200, 5, 3, 3.0f, 7.5f, 0.10f, 0.72f, 25.0f, 0.5f
};
const CharacterStats JORDI_STATS = {
    "Jordi", "strong but fragile.",
    80, 10, 5, 5.1f, 11.0f, 0.30f, 0.50f, 17.0f, 1.2f
};

static SDL_Texture* loadTex(SDL_Renderer* r, const std::string& path) {
    SDL_Texture* t = IMG_LoadTexture(r, path.c_str());
    if (!t) {
        SDL_Log("loadCharacter: failed to load '%s': %s", path.c_str(), IMG_GetError());
    }
    return t;
}

void Character::unload() {
    for (auto* t : walkFrames) {
        if (t) { SDL_DestroyTexture(t); }
    }
    for (auto* t : jumpFrames) {
        if (t) { SDL_DestroyTexture(t); }
    }
    if (idle)   { SDL_DestroyTexture(idle); }
    if (attack) { SDL_DestroyTexture(attack); }
    if (damage) { SDL_DestroyTexture(damage); }
    if (icon)   { SDL_DestroyTexture(icon); }
    walkFrames.clear();
    jumpFrames.clear();
    idle = attack = damage = icon = nullptr;
    loaded = false;
}

Character loadCharacter(SDL_Renderer* renderer, const CharacterStats& stats, const std::string& folder) {
    Character c;
    c.stats = stats;

    for (int i = 0; i < 5; ++i) {
        c.walkFrames.push_back(loadTex(renderer, folder + "/walk/" + std::to_string(i) + ".png"));
        c.jumpFrames.push_back(loadTex(renderer, folder + "/jump/" + std::to_string(i) + ".png"));
    }
    c.idle   = loadTex(renderer, folder + "/idle/0.png");
    c.attack = loadTex(renderer, folder + "/attack/0.png");
    c.damage = loadTex(renderer, folder + "/damage/0.png");
    c.icon   = loadTex(renderer, folder + "/icon.png");
    c.loaded = true;
    return c;
}