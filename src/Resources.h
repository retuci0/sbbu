#pragma once

#include "misc/Characters.h"
#include "misc/Common.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <array>


struct Resources {
    // images
    SDL_Texture* platformImage      = nullptr;
    SDL_Texture* smallPlatformImage = nullptr;
    SDL_Texture* projectileImage    = nullptr;
    SDL_Texture* shockwaveImage     = nullptr;
    SDL_Texture* heartImage         = nullptr;
    SDL_Texture* bgImage            = nullptr;
    SDL_Texture* titleBgImage       = nullptr;

    // fonts
    TTF_Font* titleFont             = nullptr;
    TTF_Font* font                  = nullptr;
    TTF_Font* smallFont             = nullptr;

    // sound effects
    Mix_Chunk* jumpSound            = nullptr;
    Mix_Chunk* jumpSound2           = nullptr;
    Mix_Chunk* deathSound           = nullptr;
    Mix_Chunk* projectileSound      = nullptr;
    Mix_Chunk* meleeSound           = nullptr;
    Mix_Chunk* parrySound           = nullptr;
    Mix_Chunk* voidDeathSound       = nullptr;
    Mix_Chunk* damageSound          = nullptr;
    Mix_Chunk* gameEndSound         = nullptr;
    Mix_Chunk** specialSounds       = nullptr;

    // music
    Mix_Music* music                = nullptr;
    Mix_Music* titleScreenMusic     = nullptr;

    // characters
    Character BERT;
    Character BERROTA;
    Character LORC;
    Character JORDI;
    Character BARCOS;
    Character ALSEXITO;
    Character SHASHA;
    Character OSCAR;
    Character FLAN;

    void load(SDL_Renderer* renderer);
    void destroy();
    TTF_Font* findFont(int size);
    void applySfxVolume(float mult);

    std::array<const Character*, CHARACTER_NUM> characterList() const {
        return { &BERT, &BERROTA, &LORC, &JORDI, &BARCOS, &ALSEXITO, &SHASHA, &OSCAR, &FLAN };
    }
};