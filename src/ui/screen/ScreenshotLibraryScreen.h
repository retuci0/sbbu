#pragma once

#include "ui/Screen.h"
#include <SDL2/SDL_rect.h>
#include <string>
#include <vector>

class ScreenshotLibraryScreen : public Screen {
public:
    ScreenshotLibraryScreen(SDL_Renderer* renderer);
    ~ScreenshotLibraryScreen();

    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;
    void update() override;

    bool isTransparent() const override { return true; }

private:
    struct Thumbnail {
        std::string path;
        SDL_Texture* texture = nullptr;
        SDL_Rect rect;
        SDL_Rect openButtonRect;
        SDL_Rect deleteButtonRect;
    };

    SDL_Renderer* renderer;
    std::vector<Thumbnail> thumbnails;
    int hoveredIndex = -1;
    int scrollOffset = 0;

    static constexpr int THUMB_W = 240;
    static constexpr int THUMB_H = 150;
    static constexpr int COLS = 7;
    static constexpr int PADDING = 20;
    static constexpr int TOP_MARGIN = 120;

    void loadThumbnails();
    void refreshLayout();
    void openScreenshot(int idx);
    void deleteScreenshot(int idx);
    void reload();
};