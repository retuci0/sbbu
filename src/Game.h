#pragma once

#include "InputHandler.h"
#include "objects/CollisionRect.h"
#include "objects/Platform.h"
#include "objects/Player.h"
#include "objects/Projectile.h"
#include "misc/Characters.h"
#include "screen/Screen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

class CharacterSelectionScreen;
class PauseScreen;
class VolumeScreen;


struct Resources {
    SDL_Texture* platformImage      = nullptr;
    SDL_Texture* smallPlatformImage = nullptr;
    SDL_Texture* projectileImage    = nullptr;
    SDL_Texture* heartImage         = nullptr;
    SDL_Texture* bgImage            = nullptr;

    TTF_Font* titleFont             = nullptr;
    TTF_Font* font                  = nullptr;
    TTF_Font* smallFont             = nullptr;

    Mix_Chunk* jumpSound            = nullptr;
    Mix_Chunk* jumpSound2           = nullptr;
    Mix_Chunk* deathSound           = nullptr;
    Mix_Chunk* projectileSound      = nullptr;
    Mix_Chunk* meleeSound           = nullptr;
    Mix_Chunk* voidDeathSound       = nullptr;
    Mix_Chunk* damageSound          = nullptr;
    Mix_Chunk* gameEndSound         = nullptr;
    Mix_Music* music                = nullptr;

    Character BERT;
    Character BERROTA;
    Character LORC;
    Character JORDI;
    Character BARCOS;
    Character ALSEXITO;
    Character SHASHA;
    Character OSCAR;

    void destroy();

    std::array<const Character*, 8> characterList() const {
        return { &BERT, &BERROTA, &LORC, &JORDI, &BARCOS, &ALSEXITO, &SHASHA, &OSCAR };
    }
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
    static constexpr int ATTACK_COOLDOWN = 20;

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    Resources     resources;

    bool running = true;
    bool debug   = false;
    bool paused  = false;

    std::unique_ptr<Screen> screen;
    void setScreen(std::unique_ptr<Screen> screen);

    std::vector<Projectile> projectiles;
    std::vector<CollisionRect> meleeHitboxes;

    Player player1, player2;
    std::vector<Platform> platforms;

    float sfxVolume   = 1.0f;
    float musicVolume = 1.0f;

    void loadResources();
    void setupPlayers(const Character* c1, const std::string& n1, const Character* c2, const std::string& n2);

    void respawn(Player& p, bool voidDeath);

    void applySfxVolume(float multiplier);
    void showEndDialog(const std::string& msg);
    
    void update();
    void updateGameplay();

    void processEvents();
    void handleGameplayInput();

    void render();
    void renderPlayerHud(const Player& player) const;
    void renderGameplay();
    void handleScreenTransitions();


    TTF_Font* findFont(int size);
};