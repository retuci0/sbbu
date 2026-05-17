// game.cpp
#include "game.h"
#include "input_handler.h"
#include "misc/common.h"
#include "misc/characters.h"
#include "screen/character_selection.h"
#include "screen/pause.h"
#include "screen/volume.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <stdexcept>
#include <random>

void Resources::destroy() {
    auto dTex = [](SDL_Texture*& t){ if (t) { SDL_DestroyTexture(t); t = nullptr; } };
    auto dChunk = [](Mix_Chunk*& c){ if (c) { Mix_FreeChunk(c); c = nullptr; } };

    dTex(platform_image); dTex(small_platform_image);
    dTex(projectile_image); dTex(heart_image); dTex(bg_image);
    if (title_font) { TTF_CloseFont(title_font); title_font = nullptr; }
    if (font)       { TTF_CloseFont(font);       font       = nullptr; }
    dChunk(jump_sound); dChunk(jump_sound2); dChunk(death_sound);
    dChunk(projectile_sound); dChunk(void_death_sound);
    dChunk(damage_sound); dChunk(game_end_sound);
    if (music) { Mix_FreeMusic(music); music = nullptr; }
    bert.unload(); berrota.unload(); lorc.unload(); jordi.unload();
}

TTF_Font* Game::findFont(int size) {
    static const char* candidates[] = {
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        TTF_Font* f = TTF_OpenFont(candidates[i], size);
        if (f) return f;
    }
    throw std::runtime_error("no TTF font found. place a font.ttf in the working directory.");
}

void Game::loadResources() {
    auto loadTex = [&](const char* path) -> SDL_Texture* {
        SDL_Texture* t = IMG_LoadTexture(renderer, path);
        if (!t) SDL_Log("missing texture: %s (%s)", path, IMG_GetError());
        return t;
    };
    auto loadChunk = [&](const char* path) -> Mix_Chunk* {
        Mix_Chunk* c = Mix_LoadWAV(path);
        if (!c) SDL_Log("missing sound: %s (%s)", path, Mix_GetError());
        return c;
    };

    resources.platform_image       = loadTex("assets/images/platform.png");
    resources.small_platform_image = loadTex("assets/images/small_platform.png");
    resources.projectile_image     = loadTex("assets/images/projectile.png");
    resources.heart_image          = loadTex("assets/images/heart.png");
    resources.bg_image             = loadTex("assets/images/background.png");

    resources.title_font = findFont(50);
    resources.font       = findFont(30);

    resources.jump_sound       = loadChunk("assets/sound/jump.wav");
    resources.jump_sound2      = loadChunk("assets/sound/jump2.wav");
    resources.death_sound      = loadChunk("assets/sound/death.wav");
    resources.projectile_sound = loadChunk("assets/sound/projectile.wav");
    resources.void_death_sound = loadChunk("assets/sound/void_death.wav");
    resources.damage_sound     = loadChunk("assets/sound/damage.wav");
    resources.game_end_sound   = loadChunk("assets/sound/game_end.wav");

    resources.music = Mix_LoadMUS("assets/sound/music.mp3");

    if (resources.jump_sound)       Mix_VolumeChunk(resources.jump_sound,       64);
    if (resources.jump_sound2)      Mix_VolumeChunk(resources.jump_sound2,      26);
    if (resources.death_sound)      Mix_VolumeChunk(resources.death_sound,      115);
    if (resources.projectile_sound) Mix_VolumeChunk(resources.projectile_sound, 26);
    if (resources.void_death_sound) Mix_VolumeChunk(resources.void_death_sound, 9);
    if (resources.damage_sound)     Mix_VolumeChunk(resources.damage_sound,     102);
    if (resources.game_end_sound)   Mix_VolumeChunk(resources.game_end_sound,   13);
    if (resources.music)            Mix_VolumeMusic(9);

    resources.bert    = loadCharacter(renderer, BERT_STATS,    "assets/images/bert");
    resources.berrota = loadCharacter(renderer, BERROTA_STATS, "assets/images/berrota");
    resources.lorc    = loadCharacter(renderer, LORC_STATS,    "assets/images/lorc");
    resources.jordi   = loadCharacter(renderer, JORDI_STATS,   "assets/images/jordi");
}

void Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        throw std::runtime_error(std::string("IMG_Init: ") + IMG_GetError());
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());
    if (TTF_Init() != 0)
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());

    window = SDL_CreateWindow(
        "super bert bros (please don't sue me nintendo)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN);
    if (!window) throw std::runtime_error(std::string("CreateWindow: ") + SDL_GetError());

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) throw std::runtime_error(std::string("CreateRenderer: ") + SDL_GetError());

    loadResources();
}

void Game::setupPlayers(const Character* c1, const std::string& n1,
                        const Character* c2, const std::string& n2) {
    platforms.clear();
    platforms.emplace_back(resources.platform_image, resources.small_platform_image,
                            360, 500, 1200, 300, PlatformSize::BIG);
    platforms.emplace_back(resources.platform_image, resources.small_platform_image,
                            640,  250,  200,  30, PlatformSize::SMALL);
    platforms.emplace_back(resources.platform_image, resources.small_platform_image,
                            1080, 250,  200,  30, PlatformSize::SMALL);

    player1.init(640, 0, c1, n1, resources.damage_sound);
    player2.init(1080, 0, c2, n2, resources.damage_sound);
}

void Game::respawn(Player& p, bool voidDeath) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> pick(0, 1);
    int spawnX = pick(rng) ? 640 : 1080;

    if (voidDeath && resources.void_death_sound) Mix_PlayChannel(-1, resources.void_death_sound, 0);
    else if (resources.death_sound)               Mix_PlayChannel(-1, resources.death_sound, 0);

    p.hp     = p.character->stats.health;
    p.rect.x = spawnX;
    p.rect.y = -2000;
    p.lives -= 1;
}

void Game::applySfxVolume(float multiplier) {
    auto set = [&](Mix_Chunk* c, int base) {
        if (c) Mix_VolumeChunk(c, std::clamp(static_cast<int>(base * multiplier), 0, 128));
    };
    set(resources.death_sound,      115);
    set(resources.projectile_sound, 26);
    set(resources.void_death_sound, 9);
    set(resources.damage_sound,     102);
    set(resources.game_end_sound,   13);
}

void Game::showEndDialog(const std::string& msg) {
    bool waiting = true;
    Uint32 startTick = SDL_GetTicks();
    while (waiting && SDL_GetTicks() - startTick < 5000) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT ||
                (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RETURN))
                waiting = false;
        }
        fillRect(renderer, 0, 0, SW, SH, 0, 0, 0, 180);
        int lineY = SH / 2 - 80;
        std::string line, rest = msg;
        while (true) {
            auto nl = rest.find('\n');
            line = (nl == std::string::npos) ? rest : rest.substr(0, nl);
            int tw, th;
            TTF_SizeText(resources.title_font, line.c_str(), &tw, &th);
            renderText(renderer, resources.title_font, line, (SW - tw) / 2, lineY, WHITE);
            lineY += th + 10;
            if (nl == std::string::npos) break;
            rest = rest.substr(nl + 1);
        }
        renderText(renderer, resources.font, "(Press Enter to close)", 800, lineY + 20, {180,180,180,255});
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

void Game::onKey(SDL_Keycode key, KeyAction action) {
    if (action != KeyAction::PRESS) return;

    if (key == K_QUIT) {
        running = false;
        return;
    }
    if (key == K_PAUSE) {
        paused = !paused;
        return;
    }
    if (key == K_HITBOX) {
        hitbox = !hitbox;
        return;
    }
    if (key == K_FULLSCREEN) {
        Uint32 flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
            SDL_SetWindowFullscreen(window, 0);
        else
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        return;
    }

    if (paused) return;

    if (key == K_P1_JUMP && action == KeyAction::PRESS) {
        player1.jump();
    } else if (key == K_P2_JUMP && action == KeyAction::PRESS) {
        player2.jump();
    } else if (key == K_P1_SHOT) {
        if (p1_cooldown > 0) return;
        if (resources.projectile_sound) Mix_PlayChannel(-1, resources.projectile_sound, 0);
        int px = (player1.facing == Facing::LEFT) ? player1.rect.x - 20 : player1.rect.x + 20;
        p1_proj.emplace_back(resources.projectile_image, resources.projectile_image,
                             px, player1.rect.y, player1.facing);
        p1_cooldown = PROJ_COOLDOWN;
    } else if (key == K_P2_SHOT) {
        if (p2_cooldown > 0) return;
        if (resources.projectile_sound) Mix_PlayChannel(-1, resources.projectile_sound, 0);
        int px = (player2.facing == Facing::LEFT) ? player2.rect.x - 20 : player2.rect.x + 20;
        p2_proj.emplace_back(resources.projectile_image, resources.projectile_image,
                             px, player2.rect.y, player2.facing);
        p2_cooldown = PROJ_COOLDOWN;
    }
}

void Game::run() {
    const Character* charArr[4] = { &resources.bert, &resources.berrota, &resources.lorc, &resources.jordi };
    auto sel = runCharacterSelection(renderer, resources.title_font, resources.font,
                                     charArr,
                                     "Player 1", &resources.bert,
                                     "Player 2", &resources.berrota);
    setupPlayers(sel.char1, sel.name1, sel.char2, sel.name2);

    if (resources.music) { Mix_PlayMusic(resources.music, -1); }

    p1_cooldown = 0;
    p2_cooldown = 0;
    p1_proj.clear();
    p2_proj.clear();

    running = true;
    while (running) {
        beginFrame();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }
            processEvent(e);
        }

        if (paused) {
            auto action = drawPauseScreen(renderer, resources.title_font, resources.font,
                                          e, SW, SH);
            switch (action) {
            case PauseAction::RESUME:
                paused = false;
                break;
            case PauseAction::QUIT:
                running = false;
                break;
            case PauseAction::CHANGE_CHARACTERS: {
                paused = false;
                auto s = runCharacterSelection(renderer, resources.title_font, resources.font,
                                               charArr,
                                               player1.name, player1.character,
                                               player2.name, player2.character);
                player1.init(player1.rect.x, player1.rect.y, s.char1, s.name1, resources.damage_sound);
                player2.init(player2.rect.x, player2.rect.y, s.char2, s.name2, resources.damage_sound);
                break;
            }
            case PauseAction::CHANGE_VOLUME: {
                paused = false;
                auto vols = runVolumeScreen(renderer, resources.title_font, resources.font,
                                            sfx_volume, music_volume);
                sfx_volume   = vols.sfx;
                music_volume = vols.music;
                applySfxVolume(sfx_volume);
                Mix_VolumeMusic(static_cast<int>(9 * music_volume));
                break;
            }
            default: break;
            }
        }

        if (!paused) {
            Mix_ResumeMusic();

            bool p1L = isDown(K_P1_LEFT);
            bool p1R = isDown(K_P1_RIGHT);
            bool p2L = isDown(K_P2_LEFT);
            bool p2R = isDown(K_P2_RIGHT);

            player1.move((p1L && p1R) ? 0 : p1L ? -1 : p1R ? 1 : 0);
            player2.move((p2L && p2R) ? 0 : p2L ? -1 : p2R ? 1 : 0);

            if (isDown(K_P1_SHOT)) player1.status = Status::ATTACKING;
            if (isDown(K_P2_SHOT)) player2.status = Status::ATTACKING;

            player1.update(platforms);
            player2.update(platforms);

            auto updateProjs = [&](std::vector<Projectile>& projs, Player& target, Player& shooter) {
                for (auto it = projs.begin(); it != projs.end(); ) {
                    if (it->rect.x >= SW || it->rect.x <= 0) {
                        it = projs.erase(it); continue;
                    }
                    if (SDL_HasIntersection(&it->rect, &target.rect)) {
                        target.getHit(it->direction);
                        target.hp -= shooter.character->stats.projectile_damage;
                        it = projs.erase(it); continue;
                    }
                    ++it;
                }
                if (projs.size() > MAX_PROJ) projs.erase(projs.begin());
            };
            updateProjs(p1_proj, player2, player1);
            updateProjs(p2_proj, player1, player2);

            if (p1_cooldown > 0) --p1_cooldown;
            if (p2_cooldown > 0) --p2_cooldown;

            auto handleDeath = [&](Player& p) {
                bool voidDeath = (p.rect.y >= SH + 100);
                bool hpDead    = (p.hp <= 0);
                if (!(voidDeath || hpDead)) return;
                if (p.lives > 0) {
                    respawn(p, voidDeath);
                } else if (p.lives == 0) {
                    if (resources.game_end_sound) Mix_PlayChannel(-1, resources.game_end_sound, 0);
                    p.lives = -1;
                }
            };
            handleDeath(player1);
            handleDeath(player2);

            if (player1.lives == -1 && player2.lives == -1) {
                showEndDialog("BOTH PLAYERS DIED\nwhat a skill issue");
                running = false;
            } else if (player1.lives == -1) {
                showEndDialog("GG!\n1st: " + player2.name + " (" + player2.character->stats.name + ")\n"
                              "2nd: " + player1.name + " (" + player1.character->stats.name + ")");
                running = false;
            } else if (player2.lives == -1) {
                showEndDialog("GG!\n1st: " + player1.name + " (" + player1.character->stats.name + ")\n"
                              "2nd: " + player2.name + " (" + player2.character->stats.name + ")");
                running = false;
            }
        } else {
            Mix_PauseMusic();
        }

        if (resources.bg_image)
            SDL_RenderCopy(renderer, resources.bg_image, nullptr, nullptr);
        else
            fillRect(renderer, 0, 0, SW, SH, 20, 20, 60, 255);

        for (auto& p : platforms) p.draw(renderer);

        player1.draw(renderer);
        player2.draw(renderer);

        for (auto& pr : p1_proj) pr.draw(renderer);
        for (auto& pr : p2_proj) pr.draw(renderer);

        if (hitbox) {
            for (auto& p : platforms) p.drawHitboxes(renderer);
            player1.drawHitboxes(renderer);
            player2.drawHitboxes(renderer);
            for (auto& pr : p1_proj) pr.drawHitboxes(renderer);
            for (auto& pr : p2_proj) pr.drawHitboxes(renderer);
        }
        if (player1.character->icon) {
            SDL_Rect iconRect1 = { 480, 840, 125, 57 };
            SDL_RenderCopy(renderer, player1.character->icon, nullptr, &iconRect1);
        }
        fillRect(renderer, 480, 950, 150, 45, 255, 255, 255, 255);
        renderText(renderer, resources.font, player1.name, 490, 955, BLACK);
        if (player1.lives >= 0) {
            renderText(renderer, resources.title_font, std::to_string(player1.hp) + " hp", 480, 900, WHITE);
            for (int i = 0; i <= player1.lives - 1; ++i)
                if (resources.heart_image) {
                    SDL_Rect heartRect1 = { 480 + i*35, 997, 30, 30 };
                    SDL_RenderCopy(renderer, resources.heart_image, nullptr, &heartRect1);
                }
        } else {
            renderText(renderer, resources.title_font, "DEAD", 480, 900, DARK_RED);
        }

        if (player2.character->icon) {
            SDL_Rect iconRect2 = { 1315, 840, 125, 57 };
            SDL_RenderCopy(renderer, player2.character->icon, nullptr, &iconRect2);
        }
        fillRect(renderer, 1315, 950, 150, 45, 255, 255, 255, 255);
        renderText(renderer, resources.font, player2.name, 1325, 955, BLACK);
        if (player2.lives >= 0) {
            renderText(renderer, resources.title_font, std::to_string(player2.hp) + " hp", 1315, 900, WHITE);
            for (int i = 0; i <= player2.lives - 1; ++i)
                if (resources.heart_image) {
                    SDL_Rect heartRect2 = {1315 + i*35, 997, 30, 30 };
                    SDL_RenderCopy(renderer, resources.heart_image, nullptr, &heartRect2);
                }
        } else {
            renderText(renderer, resources.title_font, "DEAD", 1315, 900, DARK_RED);
        }

        if (paused) {
            SDL_Event dummy{};
            drawPauseScreen(renderer, resources.title_font, resources.font, dummy, SW, SH);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }
}

void Game::cleanup() {
    resources.destroy();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}