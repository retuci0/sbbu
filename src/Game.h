#pragma once

#include "InputHandler.h"
#include "objects/CollisionRect.h"
#include "objects/Platform.h"
#include "objects/Player.h"
#include "objects/Projectile.h"
#include "misc/Characters.h"
#include "misc/Common.h"
#include "objects/Shockwave.h"
#include "ui/Screen.h"
#include "net/Network.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

class CharacterSelectionScreen;
class PauseScreen;
class VolumeScreen;
class MultiplayerModeScreen;
class RemoteSetupScreen;
class WaitingScreen;

struct Resources {
    // images
    SDL_Texture* platformImage      = nullptr;
    SDL_Texture* smallPlatformImage = nullptr;
    SDL_Texture* projectileImage    = nullptr;
    SDL_Texture* heartImage         = nullptr;
    SDL_Texture* bgImage            = nullptr;

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

    void destroy();

    std::array<const Character*, CHARACTER_NUM> characterList() const {
        return { &BERT, &BERROTA, &LORC, &JORDI, &BARCOS, &ALSEXITO, &SHASHA, &OSCAR, &FLAN };
    }
};

enum class NetworkMode { NONE, LOCAL, REMOTE_HOST, REMOTE_CLIENT };

class Game : public InputHandler {
public:
    void init();
    void run();
    void cleanup();

protected:
    void onKey(SDL_Keycode key, KeyAction action) override;

private:
    static constexpr int MAX_PROJ = 8;

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    Resources     resources;

    bool running = true;
    bool debug   = false;
    bool paused  = false;

    std::unique_ptr<Screen> screen;
    void setScreen(std::unique_ptr<Screen> screen);

    std::vector<Projectile> projectiles;
    std::vector<Shockwave> shockwaves;
    std::vector<CollisionRect> meleeHitboxes;
    std::vector<CollisionRect> specialHitboxes;

    Player player1, player2;
    std::vector<Platform> platforms;

    float sfxVolume   = 1.0f;
    float musicVolume = 1.0f;

    float fps;
    int frames;
    Uint32 lastFpsUpdate;

    void loadResources();
    void setupPlayers(const Character* c1, const std::string& n1, const Character* c2, const std::string& n2);
    void playTitleMusic();
    void playGameMusic();

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

    
    // network stuff

    NetworkMode networkMode = NetworkMode::NONE;
    std::unique_ptr<Network> network;
    uint8_t remoteInputBits = 0, prevRemoteInputBits = 0;
    uint8_t lastSentInputs = 0;
    uint32_t netFrame = 0;

    struct GameSetupPayload {
        uint8_t char1Idx, char2Idx;
        char name1[32], name2[32];
        uint8_t r1, g1, b1;
        uint8_t r2, g2, b2;
    } pendingSetup;

    bool hasPendingSetup = false;

    bool remoteIsDown(uint8_t bit) const { 
        return (remoteInputBits & bit) != 0; 
    }

    bool remoteIsPressed(uint8_t bit) const {
        return ((remoteInputBits & bit) != 0) && ((prevRemoteInputBits & bit) == 0);
    }

    void processNetworkPackets();
    void netSendStateUpdate();
    void netApplyStateUpdate(const StateUpdatePacket& sup);
    void netSendClientInputs();
};