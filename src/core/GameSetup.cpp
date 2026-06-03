#include "core/Game.h"

#include "core/Resources.h"
#include "core/InputHandler.h"
#include "misc/ScreenshotManager.h"
#include <SDL2/SDL_mixer.h>


///////////////////////////////////////
/*               SETUP               */
///////////////////////////////////////

void Game::init() {
    // init sdl and its subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        throw std::runtime_error(std::string("IMG_Init: ") + IMG_GetError());
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());
    }
    if (TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    if (SDLNet_Init() < 0) {
        throw std::runtime_error(std::string("SDLNet_Init: ") + SDLNet_GetError());
    }

    // load config
    options.loadFromFile();

    // get monitor size;
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    window = SDL_CreateWindow(
        "super bert bros ultimate",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        dm.w, dm.h, SDL_WINDOW_SHOWN);
    if (!window) throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
    Uint32 flags = SDL_GetWindowFlags(window);
    SDL_SetWindowFullscreen(window, flags | (options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (options.vsync) SDL_RenderSetVSync(renderer, 1);
    
    if (!renderer) throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
    SDL_RenderSetLogicalSize(renderer, SW, SH);

    lastFpsUpdate = SDL_GetTicks();
    Resources::get().load(renderer);

    Resources::get().applySfxVolume(options.sfxVolume);
    Mix_VolumeMusic(static_cast<int>(9 * options.musVolume));

    discord.init(DISCORD_APP_ID);

    input = Input();
    InputHandler::init();

    screenshots = new ScreenshotManager(renderer);
}

void Game::setup(const Character* c1, const std::string& n1,
                 const Character* c2, const std::string& n2,
                 const Stage& s)
{
    discord.setPresence(n1 + " vs " + n2, "super bert bros ultimate");

    platforms = s.platforms;
    stage = s;

    int x1 = s.spawnpoints.size() > 0 ? s.spawnpoints[0].x : 640;
    int x2 = s.spawnpoints.size() > 1 ? s.spawnpoints[1].x : 1080;

    player1.init(x1, 0, c1, n1);
    player1.id = 0;

    player2.init(x2, 0, c2, n2);
    player2.id = 1;

    player1.color = {100, 149, 237, 230};
    player2.color = {255,  80,  80, 230};

    remoteInputBits = 0;
    prevRemoteInputBits = 0;
    lastSentInputs = 0;
    netFrame = 0;
    lastAppliedStateFrame = 0;
    hasAppliedStateFrame = false;
    hasTargetState = false;
    targetProjectiles.clear();
    pingSequence = 0;
    pendingPingSequence = 0;
    lastPingSentTicks = 0;
    ping = -1;

    countdownTimer  = COUNTDOWN_DURATION;
    countdownActive = true;
    Mix_PlayChannel(-1, Resources::get().getSound("countdown"), 0);
}
