#pragma once

#include "input_handler.h"
#include "objects/platform.h"
#include "objects/player.h"
#include "objects/projectile.h"
#include "misc/characters.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

struct Resources {
    SDL_Texture* platform_image       = nullptr;
    SDL_Texture* small_platform_image = nullptr;
    SDL_Texture* projectile_image     = nullptr;
    SDL_Texture* heart_image          = nullptr;
    SDL_Texture* bg_image             = nullptr;

    TTF_Font* title_font = nullptr;
    TTF_Font* font       = nullptr;

    Mix_Chunk* jump_sound       = nullptr;
    Mix_Chunk* jump_sound2      = nullptr;
    Mix_Chunk* death_sound      = nullptr;
    Mix_Chunk* projectile_sound = nullptr;
    Mix_Chunk* void_death_sound = nullptr;
    Mix_Chunk* damage_sound     = nullptr;
    Mix_Chunk* game_end_sound   = nullptr;
    Mix_Music* music            = nullptr;

    Character bert;
    Character berrota;
    Character lorc;
    Character jordi;

    void destroy();
};

class Game : public InputHandler {
public:
    void init();
    void run();
    void cleanup();

protected:
    void onKey(SDL_Keycode key, KeyAction action) override;

private:
    static constexpr int SW = 1920, SH = 1080;
    static constexpr int MAX_PROJ      = 8;
    static constexpr int PROJ_COOLDOWN = 25;

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    Resources     resources;

    bool running = true;
    bool hitbox  = false;
    bool paused  = false;

    int p1_cooldown = 0;
    int p2_cooldown = 0;

    std::vector<Projectile> p1_proj;
    std::vector<Projectile> p2_proj;

    Player player1, player2;
    std::vector<Platform> platforms;

    float sfx_volume   = 1.0f;
    float music_volume = 1.0f;

    void loadResources();
    void setupPlayers(const Character* c1, const std::string& n1,
                      const Character* c2, const std::string& n2);
    void respawn(Player& p, bool voidDeath);
    void applySfxVolume(float multiplier);
    void showEndDialog(const std::string& msg);
    TTF_Font* findFont(int size);
};