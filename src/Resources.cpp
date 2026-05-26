#include "Resources.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>

#include <algorithm>
#include <stdexcept>


void Resources::load(SDL_Renderer* renderer) {
    auto loadTex = [&](const char* path) -> SDL_Texture* {
        SDL_Texture* t = IMG_LoadTexture(renderer, path);
        if (!t) SDL_Log("missing texture: %s (%s)", path, IMG_GetError());
        return t;
    };
    auto loadChunk = [&](const char* path) -> Mix_Chunk* {
        Mix_Chunk* c = Mix_LoadWAV(path);
        if (!c) SDL_Log("missing sound: %s (%s)", path, Mix_GetError());
        return c;
    };

    // load sprites
    platformImage      = loadTex("assets/images/platform/platform_big.png");
    smallPlatformImage = loadTex("assets/images/platform/platform_small.png");
    projectileImage    = loadTex("assets/images/projectile/projectile.png");
    shockwaveImage     = loadTex("assets/images/projectile/shockwave.png");
    heartImage         = loadTex("assets/images/ui/heart.png");
    bgImage            = loadTex("assets/images/ui/background.png");
    titleBgImage       = loadTex("assets/images/ui/titlescreen.png");

    // load fonts
    titleFont = findFont(50);
    font      = findFont(30);
    smallFont = findFont(21);

    // load sfxs
    jumpSound         = loadChunk("assets/sound/jump.wav");
    jumpSound2        = loadChunk("assets/sound/jump2.wav");
    deathSound        = loadChunk("assets/sound/death.wav");
    projectileSound   = loadChunk("assets/sound/projectile.wav");
    meleeSound        = loadChunk("assets/sound/punch.wav");
    voidDeathSound    = loadChunk("assets/sound/void_death.wav");
    damageSound       = loadChunk("assets/sound/damage.wav");
    gameEndSound      = loadChunk("assets/sound/game_end.wav");
    specialSounds = new Mix_Chunk*[4] {
        loadChunk("assets/sound/special_static.wav"),
        loadChunk("assets/sound/special_side.wav"),
        loadChunk("assets/sound/special_up.wav"),
        loadChunk("assets/sound/special_down.wav")
    };

    // load music
    music             = Mix_LoadMUS("assets/sound/music.mp3");
    titleScreenMusic  = Mix_LoadMUS("assets/sound/titlescreenmusic.mp3");

    // adjust volume
    if (jumpSound)        Mix_VolumeChunk(jumpSound,        64);
    if (jumpSound2)       Mix_VolumeChunk(jumpSound2,       26);
    if (deathSound)       Mix_VolumeChunk(deathSound,      115);
    if (projectileSound)  Mix_VolumeChunk(projectileSound,  26);
    if (voidDeathSound)   Mix_VolumeChunk(voidDeathSound,    9);
    if (damageSound)      Mix_VolumeChunk(damageSound,     102);
    if (gameEndSound)     Mix_VolumeChunk(gameEndSound,     13);
    if (music)            Mix_VolumeMusic(9);
    if (titleScreenMusic) Mix_VolumeMusic(9);

    // load characters
    BERT     = loadCharacter(renderer, BERT_STATS,     "assets/images/characters/bert");
    BERROTA  = loadCharacter(renderer, BERROTA_STATS,  "assets/images/characters/berrota");
    JORDI    = loadCharacter(renderer, JORDI_STATS,    "assets/images/characters/jordi");
    LORC     = loadCharacter(renderer, LORC_STATS,     "assets/images/characters/lorc");
    BARCOS   = loadCharacter(renderer, BARCOS_STATS,   "assets/images/characters/barcos");
    ALSEXITO = loadCharacter(renderer, ALSEXITO_STATS, "assets/images/characters/alsexito");
    // SHASHA, OSCAR, SASU, FLAN not yet available
}

void Resources::applySfxVolume(float multiplier) {
    auto set = [&](Mix_Chunk* c, int base) {
        if (c) Mix_VolumeChunk(c, std::clamp(static_cast<int>(base * multiplier), 0, 128));
    };
    set(deathSound,      115);
    set(projectileSound,  26);
    set(meleeSound,       10);
    set(voidDeathSound,    9);
    set(damageSound,     102);
    set(gameEndSound,     13);
    if (specialSounds) {
        for (int i = 0; i < 4; ++i) {
            set(specialSounds[i], 64);
        }
    }
}

void Resources::destroy() {
    auto dTex   = [](SDL_Texture*& t) { if (t) { SDL_DestroyTexture(t); t = nullptr; } };
    auto dChunk = [](Mix_Chunk*&   c) { if (c) { Mix_FreeChunk(c);      c = nullptr; } };

    dTex(platformImage);
    dTex(smallPlatformImage);
    dTex(projectileImage);
    dTex(shockwaveImage);
    dTex(heartImage);
    dTex(bgImage);
    dTex(titleBgImage);

    if (titleFont) { TTF_CloseFont(titleFont); titleFont = nullptr; }
    if (font)      { TTF_CloseFont(font);           font      = nullptr; }
    if (smallFont) { TTF_CloseFont(smallFont); smallFont = nullptr; }

    dChunk(jumpSound);
    dChunk(jumpSound2);
    dChunk(deathSound);
    dChunk(projectileSound);
    dChunk(meleeSound);
    dChunk(voidDeathSound);
    dChunk(damageSound);
    dChunk(gameEndSound);

    if (specialSounds) {
        for (int i = 0; i < 4; ++i) dChunk(specialSounds[i]);
        delete[] specialSounds;
        specialSounds = nullptr;
    }

    if (music) { Mix_FreeMusic(music); music = nullptr; }
    if (titleScreenMusic) { Mix_FreeMusic(titleScreenMusic); titleScreenMusic = nullptr; }

    BERT.unload();
    BERROTA.unload();
    JORDI.unload();
    LORC.unload();
    BARCOS.unload();
    ALSEXITO.unload();
    SHASHA.unload();
    OSCAR.unload();
    FLAN.unload();
}

TTF_Font* Resources::findFont(int size) {
    static const char* candidates[] = {
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        TTF_Font* f = TTF_OpenFont(candidates[i], size);
        if (f) return f;
    }
    throw std::runtime_error("no TTF font found: place a font.ttf in the working directory.");
}