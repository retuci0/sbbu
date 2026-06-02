#pragma once

#include "misc/Characters.h"
#include "misc/Common.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <array>
#include <string>
#include <unordered_map>


class Resources {
public:
    static Resources& get();

    Resources(const Resources&) = delete;
    Resources& operator=(const Resources&) = delete;

    void load(SDL_Renderer* renderer);
    void destroy();

    // access by name
    SDL_Texture* getTexture(const std::string& name) const;
    Mix_Chunk*   getSound(const std::string& name) const;

    // convenience
    TTF_Font* titleFont         = nullptr;
    TTF_Font* font              = nullptr;
    TTF_Font* smallFont         = nullptr;
    Mix_Music* gameMusic        = nullptr;
    Mix_Music* titleScreenMusic = nullptr;

    Character BERT, BERROTA, LORC, JORDI, BARCOS, ALSEXITO, SHASHA, OSCAR, FLAN;
    std::array<const Character*, CHARACTER_NUM> characterList() const;

    void applySfxVolume(float mult);

private:
    Resources() = default;
    ~Resources() = default;

    bool registerTexture(SDL_Renderer* renderer, const std::string& name, const std::string& path);
    bool registerSound(const std::string& name, const std::string& path);
    
    TTF_Font* findFont(int size);

    std::unordered_map<std::string, SDL_Texture*> textures;
    std::unordered_map<std::string, Mix_Chunk*>   sounds;
};