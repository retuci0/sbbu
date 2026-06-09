#pragma once

#include "InputHandler.h"
#include "Options.h"

#include "obj/Entity.h"
#include "obj/Player.h"
#include "obj/CollisionRect.h"
#include "obj/Items.h"

#include "misc/Discord.h"
#include "misc/ScreenshotManager.h"

#include "obj/Projectile.h"
#include "obj/Shockwave.h"
#include "obj/particle/ParticleManager.h"

#include "misc/Stages.h"
#include "misc/Characters.h"

#include "ui/ScreenStack.h"
#include "ui/screen/CheatMenuScreen.h"
#include "ui/screen/StageSelectionScreen.h"

#include "net/Network.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>

#include <SDL2/SDL_video.h>
#include <memory>
#include <string>
#include <vector>


// forward declare screen classes
class CharacterSelectionScreen;
class CheatMenuScreen;
class GameEndScreen;
class PauseScreen;
class RemoteSetupScreen;
class SettingsScreen;
class StageSelectionScreen;
class TitleScreen;
class VolumeScreen;
class WaitingScreen;

// id of the discord bot (aka app)
constexpr uint64_t DISCORD_APP_ID = 1511836048005660672;

class Game : public InputHandler {
public:
    // game stuff
    void init();
    void run();
    void cleanup();
    
    Player player1, player2;

    void trySpawnItem();

private:
    // more game stuff
    bool running = true;
    Stage stage;
    void update(float ts);
    void processEvents();
    void setup(const Character* c1, const std::string& n1, 
               const Character* c2, const std::string& n2,
               const Stage& stage);

    // SDL
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    // settings and current input
    Options       options;
    Input         input;

    // screenshots
    ScreenshotManager* screenshots;

    // discord rpc
    DiscordManager discord;

    // screens
    ScreenStack screens;
    void handleScreenTransitions();
    void showPauseScreen();
    void showTitleScreen();
    void showEndScreen(const std::string& title, const std::string& details);

    // pending stage selection result
    StageSelectionResult pendingStageResult;
    bool hasPendingStageResult = false;

    // game entities
    std::vector<std::unique_ptr<Entity>>        entities        = {};
    std::vector<std::unique_ptr<Item>>          items           = {};
    std::vector<std::unique_ptr<Platform>>      platforms       = {};
    std::vector<std::unique_ptr<GrapplePoint>>  grapplePoints   = {};
    std::vector<std::unique_ptr<Projectile>>    projectiles     = {};
    std::vector<std::unique_ptr<CollisionRect>> meleeHitboxes   = {};
    std::vector<std::unique_ptr<CollisionRect>> specialHitboxes = {};
    std::vector<std::unique_ptr<Shockwave>>     shockwaves      = {};
    ParticleManager particles = ParticleManager(&options.particles);
    static constexpr int MAX_PROJ = 8;
    
    // items
    bool itemsEnabled = true;
    float itemSpawnTimer = 0.0f;
    void resetItemSpawnTimer();

    static constexpr float ITEM_SPAWN_MIN = 600.0f;
    static constexpr float ITEM_SPAWN_MAX = 1200.0f;
    static constexpr int   MAX_LIVE_ITEMS = 3;

    // countdown and timer
    float countdownTimer = 0.0f;
    bool countdownActive = false;
    static constexpr float COUNTDOWN_DURATION = 210.0f;
    float timer = 0.0f;
    int timerDuration = 0;

    // fps related stuff
    float fps;
    int frames;
    Uint32 lastFpsUpdate;
    static constexpr int   TICK_RATE    = 20;                   // 20 tps
    static constexpr float TICK_MS      = 1000.0f / TICK_RATE;  // 50 ms
    static constexpr float TICK_SCALE   = 60.0f / TICK_RATE;    // 3.0 at 20hz

    // music
    Mix_Music* music = nullptr;
    void playTitleMusic();
    void playGameMusic();
    
    // gameplay
    void respawn(Player& p, bool voidDeath);
    void updateGameplay(float ts);

    // input
    void onKey(SDL_Keycode key, KeyAction action) override;
    void onControllerButton(SDL_GameControllerButton button, 
                            ControllerButtonAction action, int ctrl) override;
    void handleGameplayInput();
    void injectControllerNav(SDL_Event e);
    void injectNavigationKey(SDL_KeyCode key);
    struct NavRepeat {
        SDL_KeyCode lastKey = SDLK_UNKNOWN;
        Uint32 lastTime = 0;
        bool repeatActive = false;
    };
    NavRepeat navRepeat;
    SDL_GameControllerAxis lastActiveNavAxis = SDL_CONTROLLER_AXIS_INVALID;
    static constexpr Uint32 NAV_INITIAL_DELAY   = 300;  // ms before first repeat
    static constexpr Uint32 NAV_REPEAT_INTERVAL = 200;  // ms between repeats

    // render stuff
    void render(float ts, float a);
    void renderPlayerHud(const Player& player) const;
    void renderMinimap() const;
    void renderGameplay(float ts, float a);
    void renderDebug(float a, TTF_Font* font);
    void renderCountdown() const;
    void renderTimer() const;



    // network stuff

    std::unique_ptr<Network> network;
    
    NetworkMode networkMode = NetworkMode::NONE;
    uint16_t remoteInputBits = 0, prevRemoteInputBits = 0;
    uint16_t lastSentInputs = 0;
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
        uint8_t stageIdx = 0;
        std::string name1, name2;
        uint8_t r1 = 0, g1 = 0, b1 = 0;
        uint8_t r2 = 0, g2 = 0, b2 = 0;
    } pendingSetup;

    bool hasPendingSetup = false;

    bool remoteIsDown(uint16_t bit) const { 
        return (remoteInputBits & bit) != 0; 
    }

    bool remoteIsPressed(uint16_t bit) const {
        return ((remoteInputBits & bit) != 0) 
            && ((prevRemoteInputBits & bit) == 0);
    }

    void processNetworkPackets();
    void netSendStateUpdate();
    void netApplyStateUpdate(const StateUpdatePacket& sup);
    void netUpdatePing();
    void netSendClientInputs();
};