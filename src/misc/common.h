#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


// colors

constexpr SDL_Color WHITE    = {255, 255, 255, 255};
constexpr SDL_Color BLACK    = {  0,   0,   0, 255};
constexpr SDL_Color RED      = {255,   0,   0, 255};
constexpr SDL_Color GREEN    = {  0, 255,   0, 255};
constexpr SDL_Color BLUE     = {  0,   0, 255, 255};
constexpr SDL_Color DARK_RED = { 45,   0,   0, 255};


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


// ui helpers
inline SDL_Rect makeRect(int x, int y, int w, int h) { 
    return {x, y, w, h}; 
}

// hit testing
inline bool pointInRect(int px, int py, const SDL_Rect& r) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

// renders a string
void renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color);

// renders a filled rect
void fillRect(SDL_Renderer* r, int x, int y, int w, int h, Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255);

// renders an outlined rect
void outlineRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color color, int thickness = 2);

// returns a button's rect for hit testing
SDL_Rect renderButton(SDL_Renderer* r, TTF_Font* font, const std::string& text, int x, int y, int w, int h, SDL_Color bg, SDL_Color fg);