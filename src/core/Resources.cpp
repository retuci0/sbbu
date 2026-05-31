#include "core/Resources.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <stdexcept>


Resources& Resources::get() {
    static Resources instance;
    return instance;
}

bool Resources::registerTexture(SDL_Renderer* renderer, const std::string& name, const std::string& path) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) {
        SDL_Log("failed to load texture \"%s\": %s", path.c_str(), IMG_GetError());
        return false;
    }
    textures[name] = tex;
    return true;
}

bool Resources::registerSound(const std::string& name, const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        SDL_Log("failed to load sound \"%s\": %s", path.c_str(), Mix_GetError());
        return false;
    }
    sounds[name] = chunk;
    return true;
}

SDL_Texture* Resources::getTexture(const std::string& name) const {
    auto it = textures.find(name);
    if (it == textures.end()) {
        SDL_Log("texture not found: %s", name.c_str());
        return nullptr;
    }
    return it->second;
}

Mix_Chunk* Resources::getSound(const std::string& name) const {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        SDL_Log("sound not found: %s", name.c_str());
        return nullptr;
    }
    return it->second;
}

void Resources::load(SDL_Renderer* renderer) {
    // sprites
    registerTexture(renderer, "platform_big",   "assets/images/platform/platform_big.png");
    registerTexture(renderer, "platform_small", "assets/images/platform/platform_small.png");
    registerTexture(renderer, "projectile",     "assets/images/projectile/projectile.png");
    registerTexture(renderer, "shockwave",      "assets/images/projectile/shockwave.png");
    registerTexture(renderer, "heart",          "assets/images/ui/heart.png");
    registerTexture(renderer, "bg",             "assets/images/ui/background.png");
    registerTexture(renderer, "title_bg",       "assets/images/ui/titlescreen.png");
    registerTexture(renderer, "settings",       "assets/images/ui/settings.png");
    registerTexture(renderer, "3",              "assets/images/ui/3.png");
    registerTexture(renderer, "2",              "assets/images/ui/2.png");
    registerTexture(renderer, "1",              "assets/images/ui/1.png");
    registerTexture(renderer, "go",              "assets/images/ui/go.png");


    // sounds effects
    registerSound("click",        "assets/sound/click.wav");
    registerSound("jump",         "assets/sound/jump.wav");
    registerSound("jump2",        "assets/sound/jump2.wav");
    registerSound("death",        "assets/sound/death.wav");
    registerSound("projectile",   "assets/sound/projectile.wav");
    registerSound("melee",        "assets/sound/punch.wav");
    registerSound("parry",        "assets/sound/parry.wav");
    registerSound("void_death",   "assets/sound/void_death.wav");
    registerSound("damage",       "assets/sound/damage.wav");
    registerSound("block",        "assets/sound/block.wav");
    registerSound("game_end",     "assets/sound/game_end.wav");
    registerSound("special_static","assets/sound/special_static.wav");
    registerSound("special_side", "assets/sound/special_side.wav");
    registerSound("special_up",   "assets/sound/special_up.wav");
    registerSound("special_down", "assets/sound/special_down.wav");
    registerSound("select",       "assets/sound/select.wav");
    registerSound("countdown",    "assets/sound/countdown.wav");

    // music
    music            = Mix_LoadMUS("assets/sound/music.mp3");
    titleScreenMusic = Mix_LoadMUS("assets/sound/titlescreenmusic.mp3");

    // fonts
    titleFont = findFont(50);
    font      = findFont(30);
    smallFont = findFont(21);

    // default volumes (will be overridden by applySfxVolume() later)
    applySfxVolume(1.0f);

    // characters
    BERT     = loadCharacter(renderer, BERT_STATS,     "assets/images/characters/bert");
    BERROTA  = loadCharacter(renderer, BERROTA_STATS,  "assets/images/characters/berrota");
    JORDI    = loadCharacter(renderer, JORDI_STATS,    "assets/images/characters/jordi");
    LORC     = loadCharacter(renderer, LORC_STATS,     "assets/images/characters/lorc");
    BARCOS   = loadCharacter(renderer, BARCOS_STATS,   "assets/images/characters/barcos");
    ALSEXITO = loadCharacter(renderer, ALSEXITO_STATS, "assets/images/characters/alsexito");
}

void Resources::applySfxVolume(float multiplier) {
    auto setVol = [&](const std::string& name, int baseVol) {
        auto it = sounds.find(name);
        if (it != sounds.end())
            Mix_VolumeChunk(it->second, std::clamp(static_cast<int>(baseVol * multiplier), 0, 128));
    };
    
    setVol("death",        115);
    setVol("projectile",    26);
    setVol("punch",         17);
    setVol("parry",         21);
    setVol("void_death",     9);
    setVol("damage",       102);
    setVol("block",         72);
    setVol("game_end",      13);
    setVol("special_static",64);
    setVol("special_side",  64);
    setVol("special_up",    64);
    setVol("special_down",  64);
    setVol("click",         31);
    setVol("select",        32);

    if (music)            Mix_VolumeMusic(static_cast<int>(9 * multiplier));
    if (titleScreenMusic) Mix_VolumeMusic(static_cast<int>(9 * multiplier));
}

void Resources::destroy() {
    for (auto& pair : textures) {
        if (pair.second) SDL_DestroyTexture(pair.second);
    }
    textures.clear();

    for (auto& pair : sounds) {
        if (pair.second) Mix_FreeChunk(pair.second);
    }
    sounds.clear();

    if (titleFont) TTF_CloseFont(titleFont);
    if (font)      TTF_CloseFont(font);
    if (smallFont) TTF_CloseFont(smallFont);
    if (music)     Mix_FreeMusic(music);
    if (titleScreenMusic) Mix_FreeMusic(titleScreenMusic);

    BERT.unload();     BERROTA.unload(); LORC.unload();
    JORDI.unload();    BARCOS.unload();  ALSEXITO.unload();
    SHASHA.unload();   OSCAR.unload();   FLAN.unload();
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

std::array<const Character*, CHARACTER_NUM> Resources::characterList() const {
    return { &BERT, &BERROTA, &LORC, &JORDI, &BARCOS, &ALSEXITO, &SHASHA, &OSCAR, &FLAN };
}