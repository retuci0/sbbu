#pragma once

#include "../misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


// colors

constexpr Color WHITE    = { 255, 255, 255, 255 };
constexpr Color BLACK    = {   0,   0,   0, 255 };
constexpr Color RED      = { 255,   0,   0, 255 };
constexpr Color GREEN    = {   0, 255,   0, 255 };
constexpr Color BLUE     = {   0,   0, 255, 255 };
constexpr Color DARK_RED = {  45,   0,   0, 255 };


// keybinds (edit here to remap)
// look up key names here: https://wiki.libsdl.org/SDL2/SDLKeycodeLookup

constexpr SDL_Keycode K_P1_LEFT    = SDLK_a;
constexpr SDL_Keycode K_P1_RIGHT   = SDLK_d;
constexpr SDL_Keycode K_P1_JUMP    = SDLK_SPACE;
constexpr SDL_Keycode K_P1_SHOT    = SDLK_e;

constexpr SDL_Keycode K_P2_LEFT    = SDLK_LEFT;
constexpr SDL_Keycode K_P2_RIGHT   = SDLK_RIGHT;
constexpr SDL_Keycode K_P2_JUMP    = SDLK_UP;
constexpr SDL_Keycode K_P2_SHOT    = SDLK_RCTRL;

constexpr SDL_Keycode K_HITBOX     = SDLK_b;
constexpr SDL_Keycode K_FULLSCREEN = SDLK_F11;
constexpr SDL_Keycode K_PAUSE      = SDLK_ESCAPE;
constexpr SDL_Keycode K_QUIT       = SDLK_0;


enum class Facing { 
    LEFT, 
    RIGHT 
};


// hit testing
inline bool pointInRect(int px, int py, const SDL_Rect& r) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}