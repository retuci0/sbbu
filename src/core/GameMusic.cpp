#include "core/Game.h"

#include "core/Resources.h"
#include <SDL2/SDL_mixer.h>


///////////////////////////////////////
/*               MUSIC               */
///////////////////////////////////////

void Game::playTitleMusic() {
    Mix_Music* titleScreenMusic = Resources::get().titleScreenMusic;
    if (titleScreenMusic && music != titleScreenMusic) {
        Mix_HaltMusic();
        Mix_PlayMusic(Resources::get().titleScreenMusic, -1);
        music = titleScreenMusic;
    }
}

void Game::playGameMusic() {
    Mix_Music* gameMusic = Resources::get().gameMusic;
    if (gameMusic && music != gameMusic) {
        Mix_HaltMusic();
        Mix_PlayMusic(Resources::get().gameMusic, -1);
        music = gameMusic;
    }
}