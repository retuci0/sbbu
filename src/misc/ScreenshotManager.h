#pragma once

#include <SDL2/SDL_render.h>
#include <string>
#include <vector>

class ScreenshotManager {
public:
    ScreenshotManager(SDL_Renderer* renderer);
    
    // return path, or empty on fail
    std::string takeScreenshot();

    std::vector<std::string> scanAll() const;
    
    // caller must SDL_FreeSurface()
    static SDL_Surface* createThumbnail(const std::string& path, int thumbW, int thumbH);
    
private:
    SDL_Renderer* renderer;
    static constexpr Uint32 FORMAT = SDL_PIXELFORMAT_ARGB8888;
    std::string getFormattedName() const;
};