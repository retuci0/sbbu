#pragma once

#include "../misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>


// amount of characters
constexpr int CHARACTER_NUM = 9;


// colors

constexpr Color WHITE    = { 255, 255, 255, 255 };
constexpr Color BLACK    = {   0,   0,   0, 255 };
constexpr Color RED      = { 255,   0,   0, 255 };
constexpr Color GREEN    = {   0, 255,   0, 255 };
constexpr Color BLUE     = {   0,   0, 255, 255 };
constexpr Color DARK_RED = {  45,   0,   0, 255 };

// for network transmission
enum InputBit : uint8_t {
    LEFT    = 1 << 0,
    RIGHT   = 1 << 1,
    DOWN    = 1 << 2,
    JUMP    = 1 << 3,
    SHOOT   = 1 << 4,
    MELEE   = 1 << 5,
    SPECIAL = 1 << 6
};

enum class Facing { 
    LEFT, 
    RIGHT
};

enum class Direction {
    NONE,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// screen size
constexpr int SW = 1920;
constexpr int SH = 1080;


// hit testing
inline bool pointInRect(int px, int py, const SDL_Rect& r) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}