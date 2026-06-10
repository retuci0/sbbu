#include "core/Game.h"

#include "core/InputHandler.h"
#include "core/Resources.h"

#include "misc/Common.h"
#include "misc/Characters.h"
#include "entity/CollisionRect.h"
#include "ui/screen/TitleScreen.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

#include <algorithm>
#include <memory>


/////////////////////////////////////
/*               RUN               */
/////////////////////////////////////

void Game::run() {
    // start on title screen, don't play the screen change sound
    playTitleMusic();
    screens.clearAndPush(std::make_unique<TitleScreen>(), false);

    Uint32 prevTicks  = SDL_GetTicks();
    float accumulator = 0.0f;

    while (running) {
        beginFrame();

        Uint32 now = SDL_GetTicks();
        float elapsed = static_cast<float>(now - prevTicks);
        prevTicks = now;

        // cap to avoid "spiral of death" on lag spikes
        accumulator += std::min(elapsed, 200.0f);

        processEvents();

        while (accumulator >= TICK_MS) {
            update(TICK_SCALE);
            accumulator -= TICK_MS;
        }

        if (!running) return;

        float alpha = accumulator / TICK_MS;  // [0, 1)
        Uint32 frameStart = SDL_GetTicks();
        render(TICK_SCALE, alpha);
        if (options.fpsCap != -1) {
            Uint32 frameMs = 1000u / static_cast<Uint32>(options.fpsCap);
            Uint32 elapsed = SDL_GetTicks() - frameStart;
            if (elapsed < frameMs) SDL_Delay(frameMs - elapsed);
        }
    }
}


void Game::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { running = false; break; }

        if (screens) {
            screens.handle(e);
            injectControllerNav(e);
            PauseManager::paused = true;
        } else {
            PauseManager::paused = false;
            // ingame pause handling
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == options.keyPause) {
                showPauseScreen();
                continue;
            }
            if (e.type == SDL_CONTROLLERBUTTONDOWN &&
                e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                showPauseScreen();
                continue;
            }
        }
        processEvent(e);
    }
}

void Game::update(float ts) {
    // update discord rpc
    discord.update();

    if (screens) {
        screens.update();
        if (network) {
            network->poll();
            processNetworkPackets();
            netUpdatePing();
        }
        handleScreenTransitions();
        return;
    }

    // ingame
    Mix_ResumeMusic();

    if (network) {
        network->poll();
        processNetworkPackets();
        netUpdatePing();
        if (network->isConnected()
                && (networkMode == NetworkMode::REMOTE_HOST || networkMode == NetworkMode::REMOTE_CLIENT)
                && SDL_GetTicks() - network->getLastReceiveTicks() > REMOTE_TIMEOUT_MS
                && player1->lives >= 0 && player2->lives >= 0) {
            network->disconnect(false);
            if (networkMode == NetworkMode::REMOTE_HOST) {
                player2->lives = -1;
            } else {
                player1->lives = -1;
            }
        }
    }

    if (networkMode == NetworkMode::REMOTE_CLIENT) {
        netSendClientInputs();
        player1->animate(ts);
        player2->animate(ts);
        player1->updateTimers(ts);
        player2->updateTimers(ts);
    } else {
        if (countdownActive) {
            countdownTimer -= ts;
            if (countdownTimer <= 0.0f) {
                countdownTimer  = 0.0f;
                countdownActive = false;
            }
        } else {
            handleGameplayInput();
            updateGameplay(ts);
        }
        particles.update(entities, ts);
        if (networkMode == NetworkMode::REMOTE_HOST && network && network->isConnected()) {
            netSendStateUpdate();
        }
    }

    netFrame++;

    // end game
    if (timer <= 0.0f && timerDuration > 0) {
        std::string details;
        if (player1->lives > player2->lives) {
            details = player1->name + " wins!";
        } else if (player2->lives > player1->lives) {
            details = player2->name + " wins!";
        } else {
            details = "it's a tie!";
        }
        showEndScreen("time concluded!", details);
        return;
    }
    if (player1->lives == -1 && player2->lives == -1) {
        showEndScreen("both players died", "what a skill issue");
    } else if (player1->lives == -1) {
        showEndScreen("gg!", "1st: " + player2->name + " (" + player2->character.stats.name + ")   "
                            "2nd: " + player1->name + " (" + player1->character.stats.name + ")");
    } else if (player2->lives == -1) {
        showEndScreen("gg!", "1st: " + player1->name + " (" + player1->character.stats.name + ")   "
                            "2nd: " + player2->name + " (" + player2->character.stats.name + ")");
    }
}


/////////////////////////////////////////
/*               CLEANUP               */
/////////////////////////////////////////

void Game::cleanup() {
    // save to config file
    options.saveToFile();

    // disconnect
    if (network) { 
        network->disconnect(); 
        network.reset(); 
    }

    // destroy textures
    Resources::get().destroy();

    // destroy renderer and window
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window)   { SDL_DestroyWindow(window);     window   = nullptr; }

    // shutdown input handler
    InputHandler::shutdown();

    // delete screenshot manager pointer
    delete screenshots;

    // cleanup discord stuff
    discord.cleanup(); 

    // quit SDL subsystems
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}