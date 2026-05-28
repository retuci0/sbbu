#pragma once

#include "InputHandler.h"
#include "Options.h"
#include "Resources.h"

#include "objects/CollisionRect.h"
#include "objects/Platform.h"
#include "objects/Player.h"
#include "objects/Projectile.h"
#include "misc/Characters.h"
#include "objects/Shockwave.h"
#include "ui/Screen.h"
#include "net/Network.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>

#include <memory>
#include <string>
#include <vector>


class CharacterSelectionScreen;
class PauseScreen;
class VolumeScreen;
class TitleScreen;
class RemoteSetupScreen;
class WaitingScreen;
class GameEndScreen;

class Game : public InputHandler {
public:
    void init();
    void run();
    void cleanup();

protected:
    void onKey(SDL_Keycode key, KeyAction action) override;
    void onControllerButton(SDL_GameControllerButton button, ControllerButtonAction action, int ctrl) override;

private:
    static constexpr int MAX_PROJ = 8;

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    Resources     resources;
    Options       options;

    bool running = true;
    bool paused  = false;

    Input input = Input();

    std::unique_ptr<Screen> screen;
    void setScreen(std::unique_ptr<Screen> screen);

    std::vector<Projectile> projectiles;
    std::vector<Shockwave> shockwaves;
    std::vector<CollisionRect> meleeHitboxes;
    std::vector<CollisionRect> specialHitboxes;

    Player player1, player2;
    std::vector<Platform> platforms;

    float fps;
    int frames;
    Uint32 lastFpsUpdate;

    static constexpr int   TICK_RATE    = 20;                   // 20 tps
    static constexpr float TICK_MS      = 1000.0f / TICK_RATE;  // 50 ms
    static constexpr float TICK_SCALE   = 60.0f / TICK_RATE;    // 3.0 at 20hz

    void loadResources();
    void setupPlayers(const Character* c1, const std::string& n1, const Character* c2, const std::string& n2);
    void playTitleMusic();
    void playGameMusic();

    void respawn(Player& p, bool voidDeath);

    void applySfxVolume(float multiplier);
    void showEndScreen(const std::string& title, const std::string& details);
    
    void update(float ts);
    void updateGameplay(float ts);

    void processEvents();
    void handleGameplayInput();

    void render(float ts, float a);
    void renderPlayerHud(const Player& player) const;
    void renderMinimap() const;
    void renderGameplay(float ts, float a);
    void handleScreenTransitions();

    TTF_Font* findFont(int size);

    
    // network stuff

    std::unique_ptr<Network> network;
    
    NetworkMode networkMode = NetworkMode::NONE;
    uint8_t remoteInputBits = 0, prevRemoteInputBits = 0;
    uint8_t lastSentInputs = 0;
    uint32_t netFrame = 0;
    uint32_t lastAppliedStateFrame = 0;
    bool hasAppliedStateFrame = false;
    PlayerState targetPlayer1State;
    PlayerState targetPlayer2State;
    std::vector<ProjectileState> targetProjectiles;
    bool hasTargetState = false;
    uint32_t pingSequence = 0;
    uint32_t pendingPingSequence = 0;
    Uint32 lastPingSentTicks = 0;
    int ping = -1;

    struct GameSetupPayload {
        uint8_t char1Idx = 0, char2Idx = 0;
        std::string name1, name2;
        uint8_t r1 = 0, g1 = 0, b1 = 0;
        uint8_t r2 = 0, g2 = 0, b2 = 0;
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
    void netUpdatePing();
    void netSendClientInputs();
};