#include "core/Game.h"

#include "misc/Stages.h"

#include "ui/screen/CharacterSelectionScreen.h"
#include "ui/screen/ControlsScreen.h"
#include "ui/screen/GameEndScreen.h"
#include "ui/screen/PauseScreen.h"
#include "ui/screen/RemoteSetupScreen.h"
#include "ui/screen/ScreenshotLibraryScreen.h"
#include "ui/screen/SettingsScreen.h"
#include "ui/screen/StageSelectionScreen.h"
#include "ui/screen/TitleScreen.h"
#include "ui/screen/VolumeScreen.h"
#include "ui/screen/WaitingScreen.h"

#include <memory>


/////////////////////////////////////////
/*               SCREENS               */
/////////////////////////////////////////

void Game::handleScreenTransitions() {
    if (Screen* screen = screens.current(); screen && screen->hasTransition()) {
        if (screens.applyTransition(screen->takeTransition()) == ScreenAction::QUIT_GAME) {
            running = false;
        }
        return;
    }

    // title screen
    if (auto* ts = screens.currentAs<TitleScreen>()) {
        if (!ts->isFinished()) return;
        switch (ts->getResult()) {
            case TitleScreenResult::LOCAL:
                networkMode = NetworkMode::LOCAL;
                {
                    auto stages = allStages();
                    ts->resetFinished();
                    screens.push(std::make_unique<StageSelectionScreen>(stages[0], stages));
                }
                break;
            case TitleScreenResult::ONLINE:
                ts->resetFinished();
                screens.push(std::make_unique<RemoteSetupScreen>());
                break;
            case TitleScreenResult::QUIT:
                running = false;
                break;
        }
        return;
    }

    // remote setup (p2p)
    if (auto* rs = screens.currentAs<RemoteSetupScreen>()) {
        if (!rs->isFinished()) return;
        auto setupResult = rs->takeResult();
        network = std::move(setupResult.network);

        if (setupResult.role == RemoteSetupRole::HOST) {
            networkMode = NetworkMode::REMOTE_HOST;
            auto stages = allStages();
            rs->resetFinished();
            screens.push(std::make_unique<StageSelectionScreen>(stages[0], stages));
        } else {
            networkMode = NetworkMode::REMOTE_CLIENT;
            rs->resetFinished();
            screens.push(std::make_unique<WaitingScreen>());
        }
        return;
    }

    // waiting screen
    if (auto* ws = screens.currentAs<WaitingScreen>()) {
        if (hasPendingSetup && networkMode == NetworkMode::REMOTE_CLIENT) {
            hasPendingSetup = false;
            auto charList = Resources::get().characterList();
            uint8_t i1 = pendingSetup.char1Idx;
            uint8_t i2 = pendingSetup.char2Idx;
            if (!charList[i1] || !charList[i2] || !charList[i1]->loaded || !charList[i2]->loaded) {
                return;
            }
            auto stages = allStages();
            uint8_t si  = pendingSetup.stageIdx;
            if (si >= stages.size()) si = 0;
            setup(charList[i1], pendingSetup.name1, charList[i2], pendingSetup.name2, stages[si]);
            player1.color = { pendingSetup.r1, pendingSetup.g1, pendingSetup.b1, 230 };
            player2.color = { pendingSetup.r2, pendingSetup.g2, pendingSetup.b2, 230 };
            player1.resetTimers(); player2.resetTimers();
            projectiles.clear(); meleeHitboxes.clear(); specialHitboxes.clear();
            particles.clear();
            if (Resources::get().gameMusic) playGameMusic();
            screens.clear();
            return;
        }
        // keep waiting
        return;
    }

    // stage selection screen
    if (auto* ss = screens.currentAs<StageSelectionScreen>()) {
        if (!ss->isFinished()) return;

        auto stageResult = ss->getResult();
        pendingStageResult = stageResult;  // store it
        timerDuration = stageResult.time * 60;  // in ticks
        timer = timerDuration;
        hasPendingStageResult = true;

        ss->resetFinished();
        screens.push(std::make_unique<CharacterSelectionScreen>(
            Resources::get().characterList(),
            "player 1", &Resources::get().BERT,
            "player 2", &Resources::get().BERT));
        return;
    }

    // character selection screen
    if (auto* cs = screens.currentAs<CharacterSelectionScreen>()) {
        if (!cs->isFinished()) return;
        auto csResult = cs->getResult();

        if (networkMode == NetworkMode::REMOTE_HOST && (!network || !network->isConnected())) {
            return;
        }
        if (!csResult.char1 || !csResult.char2 || !csResult.char1->loaded || !csResult.char2->loaded) {
            return;
        }

        setup(csResult.char1, csResult.name1, csResult.char2, csResult.name2,
          pendingStageResult.stage);  // use stored stage
        hasPendingStageResult = false;
        player1.color = csResult.color1;
        player2.color = csResult.color2;
        player1.resetTimers();
        player2.resetTimers();
        projectiles.clear();
        meleeHitboxes.clear();
        specialHitboxes.clear();
        particles.clear();

        if (networkMode == NetworkMode::REMOTE_HOST && network && network->isConnected()) {
            auto charList = Resources::get().characterList();
            uint8_t i1 = 0, i2 = 0;
            for (int i = 0; i < CHARACTER_NUM; ++i) {
                if (charList[i] == csResult.char1) i1 = static_cast<uint8_t>(i);
                if (charList[i] == csResult.char2) i2 = static_cast<uint8_t>(i);
            }
            auto stages = allStages();
            uint8_t si = 0;
            for (int i = 0; i < static_cast<int>(stages.size()); ++i) {
                if (stages[i].name == pendingStageResult.stage.name) si = static_cast<uint8_t>(i);
            }
            GameSetupPacket gsp(i1, i2, csResult.name1, csResult.name2,
                                csResult.color1.r, csResult.color1.g, csResult.color1.b,
                                csResult.color2.r, csResult.color2.g, csResult.color2.b, si);
            network->send(gsp);
        }

        if (Resources::get().gameMusic) { playGameMusic(); }
        screens.clear();
        return;
    }

    // pause screen
    if (auto* ps = screens.currentAs<PauseScreen>()) {
        if (!ps->isFinished()) return;
        switch (ps->getResult()) {
            case PauseActionResult::RESUME:
                Mix_ResumeMusic(); Mix_Resume(-1);
                screens.pop();
                break;
            case PauseActionResult::QUIT:
                showTitleScreen();
                break;
            case PauseActionResult::RESTART:
                Mix_HaltMusic(); Mix_HaltChannel(-1);
                if (networkMode == NetworkMode::REMOTE_HOST || networkMode == NetworkMode::REMOTE_CLIENT) {
                    if (network) network->disconnect();
                    network.reset();
                    networkMode = NetworkMode::NONE;
                    showTitleScreen();
                } else {
                    auto stages = allStages();
                    showTitleScreen();
                    screens.push(std::make_unique<StageSelectionScreen>(stage, stages));
                }
                break;
            case PauseActionResult::CHANGE_VOLUME:
                ps->resetFinished();
                screens.push(std::make_unique<VolumeScreen>(
                    options.sfxVolume, options.musVolume));
                break;
            case PauseActionResult::CHANGE_CONTROLS:
                ps->resetFinished();
                screens.push(std::make_unique<ControlsScreen>(
                    options, *this));
                break;
            case PauseActionResult::SETTINGS:
                ps->resetFinished();
                screens.push(std::make_unique<SettingsScreen>(
                    options.fpsCap, options.vsync, options.fullscreen, options.debug, options.particles));
                break;
            case PauseActionResult::SCREENSHOTS:
                ps->resetFinished();
                screens.push(std::make_unique<ScreenshotLibraryScreen>(renderer));
                break;
        }
        return;
    }

    // volume screen
    if (auto* vs = screens.currentAs<VolumeScreen>()) {
        if (!vs->isFinished()) return;
        auto vols = vs->getResult();
        options.sfxVolume   = vols.sfx;
        options.musVolume = vols.music;
        Resources::get().applySfxVolume(options.sfxVolume);
        Mix_VolumeMusic(static_cast<int>(9 * options.musVolume));
        screens.goBack();
        return;
    }

    // controls screen
    if (auto* cs = screens.currentAs<ControlsScreen>()) {
        if (!cs->isFinished()) return;
        options.saveToFile();
        screens.goBack();
        return;
    }

    // video settings screen
    if (auto* ss = screens.currentAs<SettingsScreen>()) {
        if (!ss->isFinished()) return;
        auto settings = ss->getResult();
        options.fpsCap = settings.fpsCap;
        if (options.fullscreen != settings.fullscreen) {
            Uint32 flags = SDL_GetWindowFlags(window);
            SDL_SetWindowFullscreen(window,
                (settings.fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }
        if (options.vsync != settings.vsync) {
            SDL_RenderSetVSync(renderer, settings.vsync);
        }
        options.debug = settings.debug;
        options.vsync = settings.vsync;
        options.fullscreen = settings.fullscreen;
        options.particles = settings.particles;
        screens.goBack();
    }

    if (auto* ge = screens.currentAs<GameEndScreen>()) {
        if (!ge->isFinished()) return;
        if (ge->getResult() == GameEndActionResult::QUIT) {
            running = false;
            return;
        }
        showTitleScreen();
        return;
    }
}

void Game::showPauseScreen() {
    Mix_PauseMusic();
    Mix_Pause(-1);
    screens.push(std::make_unique<PauseScreen>(options));
}

void Game::showTitleScreen() {
    playTitleMusic();
    screens.clearAndPush(std::make_unique<TitleScreen>());
}

void Game::showEndScreen(const std::string& title, const std::string& details) {
    Mix_HaltMusic();
    Mix_Chunk* gameEndSound = Resources::get().getSound("game_end");
    if (gameEndSound) Mix_PlayChannel(-1, gameEndSound, 0);
    if (network) {
        network->disconnect();
        network.reset();
    }
    networkMode = NetworkMode::NONE;
    screens.clearAndPush(std::make_unique<GameEndScreen>(title, details));
}
