#include "core/Game.h"

#include "misc/Renderer.h"
#include <SDL2/SDL_ttf.h>
#include <string>


///////////////////////////////////////////
/*               RENDERING               */
///////////////////////////////////////////

void Game::render(float ts, float a) {
    if (Screen* screen = screens.current()) {
        // render the gameplay underneath on transparent screens
        if (screen->isTransparent()) {
            renderGameplay(ts, a);
        }
        screen->render(renderer);
    } else {
        renderGameplay(ts, a);
    }
    SDL_RenderPresent(renderer);

    ++frames;
    Uint32 now = SDL_GetTicks();
    if (now - lastFpsUpdate >= 1000) {
        fps           = frames * 1000.0f / static_cast<float>(now - lastFpsUpdate);
        frames        = 0;
        lastFpsUpdate = now;
    }
}

void Game::renderPlayerHud(const Player* player) const {
    int x = (player == player1) ? 480 : 1315;

    SDL_Texture* icon = (player->invulnerableTimer > 0 && player->damagedTimer == 0)
                      ? player->character.deadIcon
                      : player->character.icon;
    int w2, h2;
    SDL_QueryTexture(icon, nullptr, nullptr, &w2, &h2);
    SDL_Rect iconRect = { x, SH - 183 - h2, w2, h2 };
    Renderer::drawSprite(renderer, icon, &iconRect, false);

    Renderer::fillRect(renderer, x, 950, 150, 45, WHITE);
    Renderer::renderText(renderer, Resources::get().font, player->name, x + 10, 955, BLACK);

    if (player->lives >= 0) {
        Renderer::renderText(renderer, Resources::get().titleFont,
            std::to_string(player->hp) + " hp", x, 900, WHITE);
        for (int i = 0; i < player->lives; ++i) {
            SDL_Texture* heartImage = Resources::get().getTexture("heart");
            SDL_Rect heart = { x + i * 35, 997, 30, 30 };
            Renderer::drawSprite(renderer, heartImage, &heart, false);
        }
    } else {
        Renderer::renderText(renderer, Resources::get().titleFont, "DEAD", x, 900, DARK_RED);
    }

    Renderer::fillRect(renderer, x - 10, 840, 5, 57, BLACK);
    int h = static_cast<int>(player->charge * 57);
    Color c = { static_cast<int>(255.0f - player->charge * 255.0f),
                static_cast<int>(player->charge * 255.0f), 0 };
    Renderer::fillRect(renderer, x - 10, 840 + (57 - h), 5, h, c);

    // network indicator
    if (networkMode != NetworkMode::NONE && networkMode != NetworkMode::LOCAL) {
        bool isRemotePlayer = (networkMode == NetworkMode::REMOTE_HOST && player == player2)
                           || (networkMode == NetworkMode::REMOTE_CLIENT && player == player1);
        if (isRemotePlayer) {
            Renderer::renderText(renderer, Resources::get().smallFont, "[net]", x, 870, { 130, 200, 255, 255 });
        }
    }
}

void Game::renderMinimap() const {
    // world bounds
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

    // platforms
    for (const auto& p : platforms) {
        const SDL_Rect& r = p->rect;
        minX = std::min(minX, r.x);
        maxX = std::max(maxX, r.x + r.w);
        minY = std::min(minY, r.y);
        maxY = std::max(maxY, r.y + r.h);
    }

    // players
    auto expand = [&](const Player* player) {
        minX = std::min(minX, player->rect.x);
        maxX = std::max(maxX, player->rect.x + player->rect.w);
        minY = std::min(minY, player->rect.y);
        maxY = std::max(maxY, player->rect.y + player->rect.h);
    };
    expand(player1);
    expand(player2);

    // padding
    int padding = 100;
    minX -= padding;
    maxX += padding;
    minY -= padding;
    maxY += padding;

    float worldW = static_cast<float>(maxX - minX);
    float worldH = static_cast<float>(maxY - minY);
    if (worldW <= 0.0f || worldH <= 0.0f) return; // safety

    // minimap thingy
    Renderer::fillRect(renderer, MM_X, MM_Y, MM_W, MM_H, { 40, 40, 40, 200 });
    Renderer::outlineRect(renderer, MM_X, MM_Y, MM_W, MM_H, { 255, 255, 255, 100 }, 2);

    auto worldToMinimap = [&](float wx, float wy) -> std::pair<float, float> {
        float mx = MM_X + ((wx - minX) / worldW) * MM_W;
        float my = MM_Y + ((wy - minY) / worldH) * MM_H;
        return { mx, my };
    };

    // draw platforms
    for (const auto& p : platforms) {
        const SDL_Rect& r = p->rect;
        auto [mx1, my1] = worldToMinimap(r.x, r.y);
        auto [mx2, my2] = worldToMinimap(r.x + r.w, r.y + r.h);
        int mw = static_cast<int>(mx2 - mx1);
        int mh = static_cast<int>(my2 - my1);
        if (mw > 0 && mh > 0) {
            Renderer::fillRect(renderer, static_cast<int>(mx1), static_cast<int>(my1), mw, mh, { 101, 67, 33, 200 });
        }
    }

    // draw players with their respective color
    auto drawPlayer = [&](const Player* player, Color color) {
        auto [mx, my] = worldToMinimap(player->rect.x + player->rect.w / 2.0f,
                                       player->rect.y + player->rect.h / 2.0f);
        Renderer::fillCircle(renderer, static_cast<int>(mx), static_cast<int>(my), 2, color);
    };
    drawPlayer(player1, player1->color);
    drawPlayer(player2, player2->color);
}

void Game::renderCountdown() const {
    if (!countdownActive || countdownTimer <= 0.0f) return;

    std::string name;
    float progress = countdownTimer / COUNTDOWN_DURATION;
    if (progress > 0.75)        name = "3";
    else if (progress > 0.5)    name = "2";
    else if (progress > 0.25)   name = "1";
    else                        name = "go";

    SDL_Texture* tex = Resources::get().getTexture(name);
    int w, h;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    SDL_Rect rect = { (SW - w) / 2, (SH - h) / 2, w, h };
    Renderer::drawSprite(renderer, tex, &rect, false);
}

void Game::renderTimer() const {
    if (timer <= 0.0f || timerDuration <= 0) return;
    // format manually because i wanted to
    int seconds = static_cast<int>(timer / 60);
    int minutes = static_cast<int>(seconds / 60);
    int secs = minutes > 0 ? seconds % minutes : seconds;
    std::string s = secs < 10 ? "0" + std::to_string(secs) : std::to_string(secs);
    std::string time = std::to_string(minutes) + ":" + s;
    int w, h;
    TTF_SizeText(Resources::get().titleFont, time.c_str(), &w, &h);
    Renderer::renderText(renderer, Resources::get().titleFont, time, SW - w - 8, SH - h - 8, WHITE);
}

void Game::renderDebug(float a, TTF_Font* font) {
    for (auto& entity : entities) {
        entity->drawHitbox(renderer, a);
    }

    Renderer::renderText(renderer, font,
        player1->name + ": " + player1->getStatusName(), 2, 2, BLACK);
    Renderer::renderText(renderer, font,
        player2->name + ": " + player2->getStatusName(), 2, 32, BLACK);

    std::string fpsStr = "fps: " + std::to_string(static_cast<int>(fps));
    int tw, th;
    TTF_SizeText(font, fpsStr.c_str(), &tw, &th);
    Renderer::renderText(renderer, font, fpsStr, SW - tw - 2, 2, BLACK);

    if (networkMode == NetworkMode::REMOTE_HOST || networkMode == NetworkMode::REMOTE_CLIENT) {
        std::string pingStr = "ping: ";
        pingStr += (ping >= 0) ? std::to_string(ping) + " ms" : "? ms";
        TTF_SizeText(font, pingStr.c_str(), &tw, &th);
        Renderer::renderText(renderer, font, pingStr, SW - tw - 2, 34, BLACK);
    }
}

void Game::renderGameplay(float ts, float a) {
    TTF_Font* font = Resources::get().font;

    SDL_Texture* bgImage = Resources::get().getTexture(stage.bg);
    SDL_Rect bgRect = { 0, 0, SW, SH };
    Renderer::drawSprite(renderer, bgImage, &bgRect, false);

    for (auto& entity : entities) {
        entity->draw(renderer, a);
    }

    particles.draw(renderer, a);

    if (options.debug) renderDebug(a, font);

    renderPlayerHud(player1);
    renderPlayerHud(player2);
    
    if (player1->rect.x < -player1->rect.w || player1->rect.x > SW
            || player2->rect.x < -player2->rect.w || player2->rect.x > SW
    ) {
        renderMinimap();
    }

    renderCountdown();
    renderTimer();
}
