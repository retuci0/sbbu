#include "ui/screen/ScreenshotLibraryScreen.h"

#include "misc/ScreenshotManager.h"
#include "misc/Renderer.h"
#include "misc/Common.h"

#include <filesystem>
#include <algorithm>


ScreenshotLibraryScreen::ScreenshotLibraryScreen(SDL_Renderer* renderer)
    : renderer(renderer), Screen()
{
    loadThumbnails();
}

ScreenshotLibraryScreen::~ScreenshotLibraryScreen() {
    for (auto& t : thumbnails) {
        if (t.texture) SDL_DestroyTexture(t.texture);
    }
}

void ScreenshotLibraryScreen::reload() {
    // clean up old textures
    for (auto& t : thumbnails) {
        if (t.texture) SDL_DestroyTexture(t.texture);
    }
    thumbnails.clear();
    scrollOffset = 0;
    loadThumbnails();
}

void ScreenshotLibraryScreen::loadThumbnails() {
    ScreenshotManager mgr(renderer);
    std::vector<std::string> files = mgr.scanAll();

    // show newest first
    std::reverse(files.begin(), files.end());

    for (const auto& path : files) {
        SDL_Surface* thumbSurf = ScreenshotManager::createThumbnail(path, THUMB_W, THUMB_H);
        if (!thumbSurf) continue;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, thumbSurf);
        SDL_FreeSurface(thumbSurf);
        if (tex) {
            thumbnails.push_back({ path, tex, {0,0,THUMB_W,THUMB_H }, {0,0,0,0}, {0,0,0,0}});
        }
    }
    refreshLayout();
}

void ScreenshotLibraryScreen::refreshLayout() {
    int x = PADDING;
    int y = TOP_MARGIN - scrollOffset;
    int col = 0;
    for (auto& t : thumbnails) {
        t.rect = { x, y, THUMB_W, THUMB_H + 40 };
        int btnW = 70, btnH = 30;
        int btnY = y + THUMB_H + 5;
        t.openButtonRect   = {x + THUMB_W/2 - btnW - 5, btnY, btnW, btnH};
        t.deleteButtonRect = {x + THUMB_W/2 + 5,       btnY, btnW, btnH};
        col++;
        x += THUMB_W + PADDING;
        if (col >= COLS) {
            col = 0;
            x = PADDING;
            y += THUMB_H + PADDING + 40;
        }
    }
}

void ScreenshotLibraryScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    if (e.type == SDL_MOUSEWHEEL) {
        scrollOffset -= e.wheel.y * 30;
        if (scrollOffset < 0) scrollOffset = 0;
        refreshLayout();
    }

    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x, my = e.motion.y;
        hoveredIndex = -1;
        for (size_t i = 0; i < thumbnails.size(); ++i) {
            if (pointInRect(mx, my, thumbnails[i].rect)) {
                hoveredIndex = i;
                break;
            }
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        for (size_t i = 0; i < thumbnails.size(); ++i) {
            const auto& t = thumbnails[i];
            if (pointInRect(mx, my, t.openButtonRect)) {
                openScreenshot(i);
                return;
            }
            if (pointInRect(mx, my, t.deleteButtonRect)) {
                deleteScreenshot(i);
                return;
            }
        }
    }

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        goBack();
    }
}

void ScreenshotLibraryScreen::openScreenshot(int idx) {
    if (idx >= 0 && idx < (int)thumbnails.size()) {
        ::open(thumbnails[idx].path);
    }
}

void ScreenshotLibraryScreen::deleteScreenshot(int idx) {
    if (idx < 0 || idx >= (int)thumbnails.size()) return;
    const std::string& path = thumbnails[idx].path;
    // remove from disk
    if (std::filesystem::remove(path)) {
        // reload entire library
        reload();
    }
}

void ScreenshotLibraryScreen::render(SDL_Renderer* r) {
    Renderer::fillRect(r, 0, 0, SW, SH, {20, 20, 20, 100});
    Renderer::renderText(r, titleFont, "screenshots", 80, 40, WHITE);

    for (size_t i = 0; i < thumbnails.size(); ++i) {
        const auto& t = thumbnails[i];
        SDL_RenderCopy(r, t.texture, nullptr, &t.rect);
        Renderer::outlineRect(r, t.rect.x, t.rect.y, t.rect.w, t.rect.h, {200,200,200,255}, 1);

        // open and delete buttons
        if (hoveredIndex == (int)i) {
            Renderer::fillRect(r, t.openButtonRect.x, t.openButtonRect.y,
                               t.openButtonRect.w, t.openButtonRect.h, {60, 180, 60, 255});
            Renderer::renderText(r, smallFont, "open",
                                 t.openButtonRect.x + 12, t.openButtonRect.y + 8, WHITE);
            Renderer::fillRect(r, t.deleteButtonRect.x, t.deleteButtonRect.y,
                               t.deleteButtonRect.w, t.deleteButtonRect.h, {180, 60, 60, 255});
            Renderer::renderText(r, smallFont, "delete",
                                 t.deleteButtonRect.x + 8, t.deleteButtonRect.y + 8, WHITE);
        }
    }

    if (thumbnails.size() > COLS * 5) {
        Renderer::renderText(r, font, "scroll to see more", SW - 200, SH - 40, GRAY);
    }

    drawWidgets(r, font);
}

void ScreenshotLibraryScreen::update() {
    // autorefresh if screenshot is taken while open?
}