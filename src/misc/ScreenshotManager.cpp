#include "ScreenshotManager.h"

#include "Common.h"
#include "core/Resources.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_surface.h>

#include <ctime>
#include <filesystem>
#include <iostream>


ScreenshotManager::ScreenshotManager(SDL_Renderer* renderer) : renderer(renderer) {
    // create screenshots folder if missing
    std::filesystem::create_directory("screenshots");
}

std::string ScreenshotManager::takeScreenshot() {
    std::string path = getFormattedName();
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, SW, SH, 32, FORMAT);
    if (!surface) {
        std::cerr << "failed to create surface: " << SDL_GetError() << std::endl;
        return "";
    }
    if (SDL_RenderReadPixels(renderer, nullptr, FORMAT, surface->pixels, surface->pitch) != 0) {
        std::cerr << "failed to read pixels: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return "";
    }

    // save as BMP file
    SDL_SaveBMP(surface, path.c_str());

    // play sound
    Mix_Chunk* screenshotSound = Resources::get().getSound("screenshot");
    if (screenshotSound) Mix_PlayChannel(-1, screenshotSound, 0);

    // free surface and return the path
    SDL_FreeSurface(surface);
    return path;
}

std::string ScreenshotManager::getFormattedName() const {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    char filename[256];
    std::snprintf(filename, sizeof(filename),
        "screenshots/screenshot-%d-%d-%d_%d.%d.%d.bmp",
        1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
    return std::string(filename);
}

std::vector<std::string> ScreenshotManager::scanAll() const {
    std::vector<std::string> files;
    for (auto& entry : std::filesystem::directory_iterator("screenshots")) {
        if (entry.path().extension() == ".bmp" || entry.path().extension() == ".png")
            files.push_back(entry.path().string());
    }
    std::sort(files.begin(), files.end());  // oldest first
    return files;
}

SDL_Surface* ScreenshotManager::createThumbnail(const std::string& path, int thumbW, int thumbH) {
    SDL_Surface* original = SDL_LoadBMP(path.c_str());  // IMG_Load()?
    if (!original) return nullptr;
    SDL_Surface* thumb = SDL_CreateRGBSurface(0, thumbW, thumbH, 32, 0,0,0,0);
    if (!thumb) {
        SDL_FreeSurface(original);
        return nullptr;
    }
    SDL_BlitScaled(original, nullptr, thumb, nullptr);
    SDL_FreeSurface(original);
    return thumb;
}