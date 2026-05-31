#include "core/Game.h"

#include "ui/screen/CharacterSelectionScreen.h"
#include "ui/screen/ControlsScreen.h"
#include "ui/screen/GameEndScreen.h"
#include "ui/screen/PauseScreen.h"
#include "ui/screen/RemoteSetupScreen.h"
#include "ui/screen/SettingsScreen.h"
#include "ui/screen/TitleScreen.h"
#include "ui/screen/VolumeScreen.h"
#include "ui/screen/WaitingScreen.h"

#include <memory>


/////////////////////////////////////////
/*               SCREENS               */
/////////////////////////////////////////

void Game::setScreen(std::unique_ptr<Screen> newScreen, bool playSound) {
    screen = std::move(newScreen);
    if (playSound) Mix_PlayChannel(-1, Resources::get().getSound("select"), 0);
}

void Game::handleScreenTransitions() {
    // title screen
    if (auto* ts = dynamic_cast<TitleScreen*>(screen.get())) {
        if (!ts->isFinished()) return;
        switch (ts->getResult()) {
            case TitleScreenResult::LOCAL:
                networkMode = NetworkMode::LOCAL;
                setScreen(std::make_unique<CharacterSelectionScreen>(
                    Resources::get().characterList(),
                    "player 1", &Resources::get().BERT, "player 2", &Resources::get().BERT));
                    break;
            case TitleScreenResult::ONLINE:
                setScreen(std::make_unique<RemoteSetupScreen>());
                break;
            case TitleScreenResult::QUIT:
                running = false;
                break;
        }
        return;
    }

    // remote setup (p2p)
    if (auto* rs = dynamic_cast<RemoteSetupScreen*>(screen.get())) {
        if (!rs->isFinished()) return;
        if (rs->shouldGoBack()) {
            showTitleScreen();
            return;
        }
        auto setupResult = rs->takeResult();
        network = std::move(setupResult.network);

        if (setupResult.role == RemoteSetupRole::HOST) {
            networkMode = NetworkMode::REMOTE_HOST;
            setScreen(std::make_unique<CharacterSelectionScreen>(
                Resources::get().characterList(),
                "player 1", &Resources::get().BERT, "player 2", &Resources::get().BERT));
        } else {
            networkMode = NetworkMode::REMOTE_CLIENT;
            setScreen(std::make_unique<WaitingScreen>());
        }
        return;
    }

    // waiting screen
    if (auto* ws = dynamic_cast<WaitingScreen*>(screen.get())) {
        if (ws->shouldGoBack()) {
            setScreen(std::make_unique<RemoteSetupScreen>());
            return;
        }
        if (hasPendingSetup && networkMode == NetworkMode::REMOTE_CLIENT) {
            hasPendingSetup = false;
            auto charList = Resources::get().characterList();
            uint8_t i1 = pendingSetup.char1Idx;
            uint8_t i2 = pendingSetup.char2Idx;
            if (!charList[i1] || !charList[i2] || !charList[i1]->loaded || !charList[i2]->loaded) {
                return;
            }
            setup(charList[i1], pendingSetup.name1, charList[i2], pendingSetup.name2);
            player1.color = { pendingSetup.r1, pendingSetup.g1, pendingSetup.b1, 230 };
            player2.color = { pendingSetup.r2, pendingSetup.g2, pendingSetup.b2, 230 };
            player1.resetTimers(); player2.resetTimers();
            projectiles.clear(); meleeHitboxes.clear(); specialHitboxes.clear();
            if (Resources::get().music) playGameMusic();
            setScreen(nullptr);
            return;
        }
        // keep waiting
        return;
    }

    // character selection
    if (auto* cs = dynamic_cast<CharacterSelectionScreen*>(screen.get())) {
        if (cs->shouldGoBack()) {
            showTitleScreen();  // should sometimes go back to RemoteSetupScreen but idc for now
            return;
        }
        if (!cs->isFinished()) return;
        auto csResult = cs->getResult();

        if (networkMode == NetworkMode::REMOTE_HOST && (!network || !network->isConnected())) {
            return;
        }
        if (!csResult.char1 || !csResult.char2 || !csResult.char1->loaded || !csResult.char2->loaded) {
            return;
        }

        setup(csResult.char1, csResult.name1, csResult.char2, csResult.name2);
        player1.color = csResult.color1;
        player2.color = csResult.color2;
        player1.resetTimers();
        player2.resetTimers();
        projectiles.clear();
        meleeHitboxes.clear();
        specialHitboxes.clear();

        if (networkMode == NetworkMode::REMOTE_HOST && network && network->isConnected()) {
            auto charList = Resources::get().characterList();
            uint8_t i1 = 0, i2 = 0;
            for (int i = 0; i < CHARACTER_NUM; ++i) {
                if (charList[i] == csResult.char1) i1 = static_cast<uint8_t>(i);
                if (charList[i] == csResult.char2) i2 = static_cast<uint8_t>(i);
            }
            GameSetupPacket gsp(i1, i2, csResult.name1, csResult.name2,
                                csResult.color1.r, csResult.color1.g, csResult.color1.b,
                                csResult.color2.r, csResult.color2.g, csResult.color2.b);
            network->send(gsp);
        }

        if (Resources::get().music) { playGameMusic(); }
        setScreen(nullptr);
        return;
    }

    // pause screen
    if (auto* ps = dynamic_cast<PauseScreen*>(screen.get())) {
        if (!ps->isFinished()) return;
        switch (ps->getResult()) {
            case PauseActionResult::RESUME:
                Mix_ResumeMusic(); Mix_Resume(-1);
                setScreen(nullptr);
                break;
            case PauseActionResult::QUIT:
                showTitleScreen();
                break;
            case PauseActionResult::RESTART:
                Mix_HaltMusic(); Mix_HaltChannel(-1);
                if (networkMode == NetworkMode::REMOTE_HOST ||
                    networkMode == NetworkMode::REMOTE_CLIENT) {
                    if (network) network->disconnect();
                    network.reset();
                    networkMode = NetworkMode::NONE;
                    showTitleScreen();
                } else {
                    setScreen(std::make_unique<CharacterSelectionScreen>(
                        Resources::get().characterList(),
                        player1.name, player1.character,
                        player2.name, player2.character));
                }
                break;
            case PauseActionResult::CHANGE_VOLUME:
                setScreen(std::make_unique<VolumeScreen>(
                    options.sfxVolume, options.musVolume));
                break;
            case PauseActionResult::CHANGE_CONTROLS:
                setScreen(std::make_unique<ControlsScreen>(
                    options));
                break;
            case PauseActionResult::SETTINGS:
                setScreen(std::make_unique<SettingsScreen>(
                    options.fpsCap, options.vsync, options.fullscreen, options.debug));
                break;
        }
        return;
    }

    // volume screen
    if (auto* vs = dynamic_cast<VolumeScreen*>(screen.get())) {
        if (!vs->isFinished()) return;
        auto vols = vs->getResult();
        options.sfxVolume   = vols.sfx;
        options.musVolume = vols.music;
        Resources::get().applySfxVolume(options.sfxVolume);
        Mix_VolumeMusic(static_cast<int>(9 * options.musVolume));
        showPauseScreen();
        return;
    }

    if (auto* cs = dynamic_cast<ControlsScreen*>(screen.get())) {
        if (!cs->isFinished()) return;
        options.saveToFile();
        showPauseScreen();
        return;
    }

    if (auto* ss = dynamic_cast<SettingsScreen*>(screen.get())) {
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
        showPauseScreen();
    }

    if (auto* ge = dynamic_cast<GameEndScreen*>(screen.get())) {
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
    setScreen(std::make_unique<PauseScreen>(options));
}

void Game::showTitleScreen() {
    playTitleMusic();
    setScreen(std::make_unique<TitleScreen>());
}

void Game::showEndScreen(const std::string& title, const std::string& details) {
    Mix_HaltMusic();
    if (network) {
        network->disconnect();
        network.reset();
    }
    networkMode = NetworkMode::NONE;
    setScreen(std::make_unique<GameEndScreen>(title, details));
}
