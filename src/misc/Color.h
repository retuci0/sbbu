#pragma once

#include <SDL2/SDL_pixels.h>


struct Color {
    // components
    int r, g, b, a;

    constexpr Color()
    : r(0), g(0), b(0), a(255) {}

    constexpr Color(int r, int g, int b, int a)
    : r(r), g(g), b(b), a(a) {}

    constexpr Color(int r, int g, int b)
    : r(r), g(g), b(b), a(255) {}

    Color(SDL_Color c) {
        r = c.r;
        g = c.g;
        b = c.b;
        a = c.a;
    }

    constexpr Color(int argb) {
        a = (argb >> 24) & 0xFF;
        r = (argb >> 16) & 0xFF;
        g = (argb >>  8) & 0xFF;
        b = (argb >>  0) & 0xFF;
    }

    
    // ARGB integer
    constexpr int getARGB() const {
        return (a << 24) 
             | (r << 16) 
             | (g <<  8) 
             | (b <<  0);
    }

    // convert to SDL_Color type
    SDL_Color toSdlColor() const {
        return SDL_Color {
            static_cast<Uint8>(r),
            static_cast<Uint8>(g),
            static_cast<Uint8>(b),
            static_cast<Uint8>(a)
        };
    }
};