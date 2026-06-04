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

namespace {
    template<typename T>
    Character* makeAndLoad(SDL_Renderer* renderer, const std::string& folder) {
        auto* c = new T();
        loadCharacter(*c, renderer, folder);
        return c;
    }
}

void Resources::load(SDL_Renderer* renderer) {
    // sprites
    registerTexture(renderer, "platform_big",    "assets/images/platform/platform_big.png");
    registerTexture(renderer, "platform_small",  "assets/images/platform/platform_small.png");

    registerTexture(renderer, "projectile",      "assets/images/projectile/projectile.png");
    registerTexture(renderer, "shockwave",       "assets/images/projectile/shockwave.png");
    registerTexture(renderer, "grapple",         "assets/images/projectile/grapple.png");

    registerTexture(renderer, "heart",           "assets/images/ui/heart.png");
    registerTexture(renderer, "settings",        "assets/images/ui/settings.png");
    registerTexture(renderer, "screenshots",     "assets/images/ui/screenshots.png");

    registerTexture(renderer, "3",               "assets/images/ui/3.png");
    registerTexture(renderer, "2",               "assets/images/ui/2.png");
    registerTexture(renderer, "1",               "assets/images/ui/1.png");
    registerTexture(renderer, "go",              "assets/images/ui/go.png");

    registerTexture(renderer, "title_bg",        "assets/images/ui/titlescreen.png");
    registerTexture(renderer, "bg_classic",      "assets/images/ui/bg_classic.png");
    registerTexture(renderer, "bg_pillar",       "assets/images/ui/bg_pillar.png");
    registerTexture(renderer, "bg_flat",         "assets/images/ui/bg_flat.png");
    registerTexture(renderer, "bg_flat",         "assets/images/bg_flat.png");
    registerTexture(renderer, "bg_dash",         "assets/images/ui/bg_dash.png");

    registerTexture(renderer, "particle_crit",   "assets/images/particle/critical.png");
    registerTexture(renderer, "particle_damage", "assets/images/particle/damage.png");
    registerTexture(renderer, "particle_dj",     "assets/images/particle/double_jump.png");
    registerTexture(renderer, "particle_death",  "assets/images/particle/death.png");

    // sound effects
    registerSound("click",         "assets/sound/click.wav");
    registerSound("select",        "assets/sound/select.wav");
    registerSound("screenshot",    "assets/sound/screenshot.wav");

    registerSound("jump",          "assets/sound/jump.wav");
    registerSound("jump2",         "assets/sound/jump2.wav");
    registerSound("dash",          "assets/sound/dash.wav");
    registerSound("melee",         "assets/sound/punch.wav");
    registerSound("projectile",    "assets/sound/projectile.wav");
    registerSound("special_static","assets/sound/special_static.wav");
    registerSound("special_side",  "assets/sound/special_side.wav");
    registerSound("special_up",    "assets/sound/special_up.wav");
    registerSound("special_down",  "assets/sound/special_down.wav");
    registerSound("parry",         "assets/sound/parry.wav");
    registerSound("damage",        "assets/sound/damage.wav");
    registerSound("block",         "assets/sound/block.wav");
    registerSound("shield_break",  "assets/sound/break.wav");
    registerSound("death",         "assets/sound/death.wav");
    registerSound("void_death",    "assets/sound/void_death.wav");

    registerSound("countdown",     "assets/sound/countdown.wav");
    registerSound("game_end",      "assets/sound/game_end.wav");

    // music
    gameMusic        = Mix_LoadMUS("assets/sound/music.mp3");
    titleScreenMusic = Mix_LoadMUS("assets/sound/titlescreenmusic.mp3");

    // fonts
    titleFont = findFont(50);
    font      = findFont(30);
    smallFont = findFont(21);

    applySfxVolume(1.0f);

    // characters
    BERT     = std::unique_ptr<Character>(makeAndLoad<BertCharacter>    (renderer, "assets/images/characters/bert"));
    BERROTA  = std::unique_ptr<Character>(makeAndLoad<BerrotaCharacter> (renderer, "assets/images/characters/berrota"));
    JORDI    = std::unique_ptr<Character>(makeAndLoad<JordiCharacter>   (renderer, "assets/images/characters/jordi"));
    LORC     = std::unique_ptr<Character>(makeAndLoad<LorcCharacter>    (renderer, "assets/images/characters/lorc"));
    BARCOS   = std::unique_ptr<Character>(makeAndLoad<BarcosCharacter>  (renderer, "assets/images/characters/barcos"));
    ALSEXITO = std::unique_ptr<Character>(makeAndLoad<AlsexitoCharacter>(renderer, "assets/images/characters/alsexito"));
    // SHASHA, OSCAR, FLAN: uncomment when asset folders are ready
    // FLAN  = std::unique_ptr<Character>(makeAndLoad<FlanCharacter>(renderer, "assets/images/characters/flan"));
}

void Resources::applySfxVolume(float multiplier) {
    auto setVol = [&](const std::string& name, int baseVol) {
        auto it = sounds.find(name);
        if (it != sounds.end())
            Mix_VolumeChunk(it->second, std::clamp(static_cast<int>(baseVol * multiplier), 0, 128));
    };

    setVol("death",         115);
    setVol("projectile",     26);
    setVol("melee",          17);
    setVol("parry",          21);
    setVol("void_death",      9);
    setVol("damage",        102);
    setVol("block",          72);
    setVol("game_end",       13);
    setVol("special_static", 64);
    setVol("special_side",   64);
    setVol("special_up",     64);
    setVol("special_down",   64);
    setVol("click",          31);
    setVol("select",         32);
    setVol("shield_break",   20);

    if (gameMusic)        Mix_VolumeMusic(static_cast<int>(9 * multiplier));
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
    if (gameMusic)        Mix_FreeMusic(gameMusic);
    if (titleScreenMusic) Mix_FreeMusic(titleScreenMusic);

    // unload SDL textures before unique_ptr destructor runs
    auto unload = [](std::unique_ptr<Character>& c) { if (c) c->unload(); };
    unload(BERT);     unload(BERROTA); unload(LORC);
    unload(JORDI);    unload(BARCOS);  unload(ALSEXITO);
    unload(SHASHA);   unload(OSCAR);   unload(FLAN);
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
    return { BERT.get(), BERROTA.get(), LORC.get(), JORDI.get(),
             BARCOS.get(), ALSEXITO.get(), SHASHA.get(), OSCAR.get(), FLAN.get() };
}