#include "core/Game.h"

#include "core/Resources.h"


///////////////////////////////////////
/*               MUSIC               */
///////////////////////////////////////

void Game::playTitleMusic() {
    if (Resources::get().titleScreenMusic) {
        Mix_HaltMusic();
        Mix_PlayMusic(Resources::get().titleScreenMusic, -1);
    }
}

void Game::playGameMusic() {
    if (Resources::get().music) {
        Mix_HaltMusic();
        Mix_PlayMusic(Resources::get().music, -1);
    }
}