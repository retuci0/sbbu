#pragma once

#include "misc/Color.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>

#include <string>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#else
    #include <cstdlib>
#endif


// amount of characters
constexpr int CHARACTER_NUM = 9;

// protocol version of network system
static constexpr int PROTOCOL_VERSION = 1;


// colors
constexpr Color WHITE           = { 255, 255, 255, 255 };
constexpr Color GRAY            = { 100, 100, 100, 255 };
constexpr Color BLACK           = {   0,   0,   0, 255 };
constexpr Color RED             = { 255,   0,   0, 255 };
constexpr Color GREEN           = {  60, 100,  60, 255 };
constexpr Color BLUE            = {   0,   0, 255, 255 };
constexpr Color DARK_RED        = {  45,   0,   0, 255 };
constexpr Color MAGENTA         = { 255,   0, 255, 255 };


// for network transmission
enum InputBit : uint16_t {
    LEFT    = 1 << 0,
    RIGHT   = 1 << 1,
    DOWN    = 1 << 2,
    JUMP    = 1 << 3,
    SHOOT   = 1 << 4,
    MELEE   = 1 << 5,
    SPECIAL = 1 << 6,
    SHIELD  = 1 << 7,
    DASH    = 1 << 8
};

// timeout to connect to host
static constexpr Uint32 REMOTE_TIMEOUT_MS = 5000;

// represents a side
enum class Facing { 
    LEFT, 
    RIGHT
};

// represents a direction
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

// minimap constants
static constexpr int PADDING = 2;
static constexpr int MM_W = 200;
static constexpr int MM_H = 120;
static constexpr int MM_X = PADDING;
static constexpr int MM_Y = SH - MM_H - PADDING;


// hit testing
inline bool pointInRect(int px, int py, const SDL_Rect& r) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

inline SDL_Rect interpolatedRect(SDL_Rect prevRect, SDL_Rect rect, float alpha) {
    return {
        static_cast<int>(prevRect.x + (rect.x - prevRect.x) * alpha),
        static_cast<int>(prevRect.y + (rect.y - prevRect.y) * alpha),
        rect.w,
        rect.h
    };
}

// open a folder with the user's preferred file manager
inline void openFolder(const std::string& path) {
#if defined _WIN32
    ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    std::string command = "xdg-open \"" + path + "\"";
    std::system(command.c_str());
#endif
}