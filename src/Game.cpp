#include "Game.h"
#include "InputHandler.h"

#include "misc/Common.h"
#include "misc/Characters.h"
#include "misc/Renderer.h"
#include "ui/screen/CharacterSelectionScreen.h"
#include "ui/screen/PauseScreen.h"
#include "ui/screen/VolumeScreen.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <stdexcept>
#include <random>
#include <uuid/uuid.h>


void Resources::destroy() {
    auto dTex   = [](SDL_Texture*& t) { if (t) { SDL_DestroyTexture(t); t = nullptr; }};
    auto dChunk = [](Mix_Chunk*&   c) { if (c) { Mix_FreeChunk(c);        c = nullptr; }};

    // destroy images
    dTex(platformImage); 
    dTex(smallPlatformImage);
    dTex(projectileImage); 
    dTex(heartImage); 
    dTex(bgImage);

    // destroy fonts
    if (titleFont) { 
        TTF_CloseFont(titleFont); 
        titleFont = nullptr; 
    }
    if (font) { 
        TTF_CloseFont(font);      
        font = nullptr; 
    }
    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }

    // destroy sounds
    dChunk(jumpSound); 
    dChunk(jumpSound2); 
    dChunk(deathSound);
    dChunk(projectileSound); 
    dChunk(meleeSound);
    dChunk(voidDeathSound);
    dChunk(damageSound); 
    dChunk(gameEndSound);
    for (int i = 0; i < 4; i++) { 
        dChunk(specialSounds[i]); 
    }
    delete[] specialSounds;

    // destroy music
    if (music) { Mix_FreeMusic(music); music = nullptr; }

    // destroy character sprites
    BERT.unload(); 
    BERROTA.unload(); 
    JORDI.unload(); 
    LORC.unload();
    BARCOS.unload(); 
    ALSEXITO.unload(); 
    SHASHA.unload(); 
    OSCAR.unload();
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
        if (f) { return f; }
    }
    throw std::runtime_error("no TTF font found: place a `font.ttf` in the working directory.");
}

void Game::loadResources() {
    auto loadTex = [&](const char* path) -> SDL_Texture* {
        SDL_Texture* t = IMG_LoadTexture(renderer, path);
        if (!t) { SDL_Log("missing texture: %s (%s)", path, IMG_GetError()); }
        return t;
    };
    auto loadChunk = [&](const char* path) -> Mix_Chunk* {
        Mix_Chunk* c = Mix_LoadWAV(path);
        if (!c) { SDL_Log("missing sound: %s (%s)", path, Mix_GetError()); }
        return c;
    };

    resources.platformImage      = loadTex("assets/images/platform/platform_big.png");
    resources.smallPlatformImage = loadTex("assets/images/platform/platform_small.png");
    resources.projectileImage    = loadTex("assets/images/projectile/projectile.png");
    resources.heartImage         = loadTex("assets/images/ui/heart.png");
    resources.bgImage            = loadTex("assets/images/ui/background.png");

    resources.titleFont = findFont(50);
    resources.font      = findFont(30);
    resources.smallFont = findFont(21);

    resources.jumpSound       = loadChunk("assets/sound/jump.wav");
    resources.jumpSound2      = loadChunk("assets/sound/jump2.wav");
    resources.deathSound      = loadChunk("assets/sound/death.wav");
    resources.projectileSound = loadChunk("assets/sound/projectile.wav");
    resources.meleeSound      = loadChunk("assets/sound/punch.wav");
    resources.voidDeathSound  = loadChunk("assets/sound/void_death.wav");
    resources.damageSound     = loadChunk("assets/sound/damage.wav");
    resources.gameEndSound    = loadChunk("assets/sound/game_end.wav");

    resources.music = Mix_LoadMUS("assets/sound/music.mp3");

    resources.specialSounds   = new Mix_Chunk*[] { 
        loadChunk("assets/sound/special_static.wav"),
        loadChunk("assets/sound/special_side.wav"),
        loadChunk("assets/sound/special_up.wav"),
        loadChunk("assets/sound/special_down.wav")
    };

    if (resources.jumpSound)       { Mix_VolumeChunk(resources.jumpSound,        64); }
    if (resources.jumpSound2)      { Mix_VolumeChunk(resources.jumpSound2,       26); }
    if (resources.deathSound)      { Mix_VolumeChunk(resources.deathSound,      115); }
    if (resources.projectileSound) { Mix_VolumeChunk(resources.projectileSound,  26); }
    if (resources.voidDeathSound)  { Mix_VolumeChunk(resources.voidDeathSound,    9); }
    if (resources.damageSound)     { Mix_VolumeChunk(resources.damageSound,     102); }
    if (resources.gameEndSound)    { Mix_VolumeChunk(resources.gameEndSound,     13); }
    if (resources.music)           { Mix_VolumeMusic(9); }

    resources.BERT     = loadCharacter(renderer, BERT_STATS,    "assets/images/characters/bert");
    resources.BERROTA  = loadCharacter(renderer, BERROTA_STATS, "assets/images/characters/berrota");
    resources.JORDI    = loadCharacter(renderer, JORDI_STATS,    "assets/images/characters/jordi");
    resources.LORC     = loadCharacter(renderer, LORC_STATS,   "assets/images/characters/lorc");
    resources.BARCOS   = loadCharacter(renderer, BARCOS_STATS, "assets/images/characters/barcos");
    resources.ALSEXITO = loadCharacter(renderer, ALSEXITO_STATS, "assets/images/characters/alsexito");
}

void Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        throw std::runtime_error(std::string("IMG_Init: ") + IMG_GetError());
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());
    }
    if (TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }

    window = SDL_CreateWindow(
        "super bert bros (please don't sue me nintendo)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN);
    if (!window) { throw std::runtime_error(std::string("CreateWindow: ") + SDL_GetError()); }

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { throw std::runtime_error(std::string("CreateRenderer: ") + SDL_GetError()); }
    SDL_RenderSetLogicalSize(renderer, SW, SH);

    loadResources();
}

void Game::setupPlayers(const Character* c1, const std::string& n1, const Character* c2, const std::string& n2) {
    platforms.clear();
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                            360, 500, 1200, 300, PlatformSize::BIG);
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                            640,  250, 200, 30, PlatformSize::SMALL);
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                            1080, 250, 200, 30, PlatformSize::SMALL);

    player1.init(640, 0, c1, n1, resources.damageSound);
    uuid_generate(player1.uuid);

    player2.init(1080, 0, c2, n2, resources.damageSound);
    uuid_generate(player2.uuid);

    player1.color = {100, 149, 237, 230};
    player2.color = {255,  80,  80, 230};
}

void Game::respawn(Player& p, bool voidDeath) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> pick(0, 1);
    int spawnX = pick(rng) ? 640 : 1080;

    if (voidDeath && resources.voidDeathSound) {
        Mix_PlayChannel(-1, resources.voidDeathSound, 0);
    } else if (resources.deathSound) {
        Mix_PlayChannel(-1, resources.deathSound, 0);
    }

    p.hp = p.character->stats.health;
    p.rect.x = spawnX;
    p.rect.y = -2000;
    p.lives -= 1;
    p.invulnerableTimer = Player::INV_DURATION;
    p.charge = 0.0f;
    p.dx = 0.0f;
    p.dy = 0.0f;
    p.status = Status::IDLE;
    p.onGround = false;
    p.hasAirJumped = false;
    p.currentSpriteIndex = 0.0f;
    p.specialHitboxSpawned = false;
    p.resetTimers();
}

void Game::applySfxVolume(float multiplier) {
    auto set = [&](Mix_Chunk* c, int base) {
        if (c) { Mix_VolumeChunk(c, std::clamp(static_cast<int>(base * multiplier), 0, 128)); }
    };
    set(resources.deathSound,      115);
    set(resources.projectileSound,  26);
    set(resources.meleeSound,       10);
    set(resources.voidDeathSound,    9);
    set(resources.damageSound,     102);
    set(resources.gameEndSound,     13);
    if (resources.specialSounds) {
        for (int i = 0; i < 4; ++i) {
            set(resources.specialSounds[i], 64);
        }
    }
}

void Game::showEndDialog(const std::string& msg) {
    bool waiting = true;
    Uint32 startTick = SDL_GetTicks();
    while (waiting && SDL_GetTicks() - startTick < 5000) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                waiting = false;
            }
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RETURN) {
                waiting = false;
            }
        }
        Renderer::fillRect(renderer, 0, 0, SW, SH, { 0, 0, 0, 180 });
        int lineY = SH / 2 - 80;
        std::string line, rest = msg;
        while (true) {
            auto nl = rest.find('\n');
            line = (nl == std::string::npos) ? rest : rest.substr(0, nl);
            int textW, textH;
            TTF_SizeText(resources.titleFont, line.c_str(), &textW, &textH);
            Renderer::renderText(renderer, resources.titleFont, line, (SW - textW) / 2, lineY, WHITE);
            lineY += textH + 10;
            if (nl == std::string::npos) { break; }
            rest = rest.substr(nl + 1);
        }
        Renderer::renderText(renderer, resources.font, "(press enter to close)", 800, lineY + 20, {180, 180, 180, 255});
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

// gameplay input: movement and actions (only with no screen)
void Game::handleGameplayInput() {
    bool p1Left = isDown(K_P1_LEFT);
    bool p1Right = isDown(K_P1_RIGHT);
    bool p2Left = isDown(K_P2_LEFT);
    bool p2Right = isDown(K_P2_RIGHT);
    
    if (player1.status != Status::DAMAGED) {
        player1.move((p1Left && p1Right) ? 0 : p1Left ? -1 : p1Right ? 1 : 0);
    }
    if (player2.status != Status::DAMAGED) {
        player2.move((p2Left && p2Right) ? 0 : p2Left ? -1 : p2Right ? 1 : 0);
    }

    if (isPressed(K_P1_JUMP)) { player1.jump(); }
    if (isPressed(K_P2_JUMP)) { player2.jump(); }

    if (isPressed(K_P1_SHOOT)) {
        bool shot = player1.tryShoot(resources.projectileSound);
        if (shot) {
            int px = (player1.facing == Facing::LEFT) ? player1.rect.x - 20 : player1.rect.x + 20;
            projectiles.emplace_back(resources.projectileImage, px, player1.rect.y, player1.facing, &player1);
        }
    }
    if (isPressed(K_P2_SHOOT)) {
        bool shot = player2.tryShoot(resources.projectileSound);
        if (shot) {
            int px = (player2.facing == Facing::LEFT) ? player2.rect.x - 20 : player2.rect.x + 20;
            projectiles.emplace_back(resources.projectileImage, px, player2.rect.y, player2.facing, &player2);
       }
    }

    if (isPressed(K_P1_MELEE)) {
        if (player1.tryMelee(resources.meleeSound)) {
            int hx = player1.rect.x + (player1.facing == Facing::RIGHT ? player1.rect.w - 64 : 0);
            int hy = player1.rect.y + player1.rect.h - 64;
            meleeHitboxes.emplace_back(hx, hy, 64, 64, &player1, 5);
        }
    }
    if (isPressed(K_P2_MELEE)) {
        if (player2.tryMelee(resources.meleeSound)) {
            int hx = player2.rect.x + (player2.facing == Facing::RIGHT ? player2.rect.w - 64 : 0);
            int hy = player2.rect.y + player2.rect.h - 64;
            meleeHitboxes.emplace_back(hx, hy, 64, 64, &player2, 5);
        }
    }

    if (isPressed(K_P1_SPECIAL)) {
        Direction dir;
        if (isDown(K_P1_LEFT)) {
            dir = Direction::LEFT;
        } else if (isDown(K_P1_RIGHT)) {
            dir = Direction::RIGHT;
        } else if (isDown(K_P1_JUMP)) {
            dir = Direction::UP;
        } else if (isDown(K_P1_DOWN)) {
            dir = Direction::DOWN;
        } else {
            dir = Direction::NONE;
        }
        if (player1.trySpecial(resources.specialSounds, dir)) {
            // ...
        }
    }
    if (isPressed(K_P2_SPECIAL)) {
        Direction dir;
        if (isDown(K_P2_LEFT)) {
            dir = Direction::LEFT;
        } else if (isDown(K_P2_RIGHT)) {
            dir = Direction::RIGHT;
        } else if (isDown(K_P2_JUMP)) {
            dir = Direction::UP;
        } else if (isDown(K_P2_DOWN)) {
            dir = Direction::DOWN;
        } else {
            dir = Direction::NONE;
        }
        if (player2.trySpecial(resources.specialSounds, dir)) {
            // ...
        }
    }
}

void Game::updateGameplay() {
    player1.update(platforms, isDown(K_P1_DOWN));
    player2.update(platforms, isDown(K_P2_DOWN));

    // update projectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->move();
        // out of bounds
        if (it->rect.x >= SW || it->rect.x <= 0) {
            it = projectiles.erase(it);
            continue;
        }
        // check for collisions
        if (it->owner != &player1 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer > 0) { ++it; continue; };
            player1.getHit(it->direction, it->owner->character->stats.projectileDamage);
            it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
            it = projectiles.erase(it);
            continue;
        }
        if (it->owner != &player2 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer > 0) { ++it; continue; };
            player2.getHit(it->direction, it->owner->character->stats.projectileDamage);
            it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
            it = projectiles.erase(it);
            continue;
        }
        ++it;
    }
    // limit total projectiles
    if (projectiles.size() > MAX_PROJ) projectiles.erase(projectiles.begin());

    // update collision rects of melee attacks
    for (auto it = meleeHitboxes.begin(); it != meleeHitboxes.end(); ) {
        it->update();
        if (!it->isAlive()) {
            it = meleeHitboxes.erase(it);
            continue;
        }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            player2.getHit(player1.facing, it->owner->character->stats.damage);
            it = meleeHitboxes.erase(it);
            player1.charge = std::min(player1.charge + 0.1f, Player::MAX_CHARGE);
            continue;
        }
        if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            player1.getHit(player2.facing, it->owner->character->stats.damage);
            it = meleeHitboxes.erase(it);
            player2.charge = std::min(player2.charge + 0.1f, Player::MAX_CHARGE);
            continue;
        }
        ++it;
    }

    // spawn special hitbox on last animation frame
    auto trySpawnSpecialHitbox = [&](Player& attacker) {
        // do nothing if no special is active, or hitbox already spawned
        if (attacker.status != Status::SPECIAL_STATIC &&
            attacker.status != Status::SPECIAL_SIDE &&
            attacker.status != Status::SPECIAL_UP &&
            attacker.status != Status::SPECIAL_DOWN) {
            return;
        }
        if (attacker.specialHitboxSpawned) return;

        if (attacker.specialTimer > Player::SPECIAL_DURATION - 5) return;

        attacker.specialHitboxSpawned = true;

        int hx, hy, hw, hh;
        float dmgScale = 3.0f;
        float kbScale  = 5.0f;

        switch (attacker.status) {
            case Status::SPECIAL_STATIC:
                // hitbox in front of the player
                hw = 110;
                hh = attacker.rect.h + 20;
                hx = (attacker.facing == Facing::RIGHT)
                    ? attacker.rect.x + attacker.rect.w - 20
                    : attacker.rect.x - hw + 20;
                hy = attacker.rect.y - 10;
                break;
            case Status::SPECIAL_SIDE:
                // hitbox in front of the player
                hw = 130;
                hh = attacker.rect.h;
                hx = (attacker.facing == Facing::RIGHT)
                    ? attacker.rect.x + attacker.rect.w - 30
                    : attacker.rect.x - hw + 30;
                hy = attacker.rect.y;
                dmgScale = 2.5f;
                kbScale  = 4.0f;
                break;
            case Status::SPECIAL_UP:
                // hitbox above the player
                hw = attacker.rect.w + 20;
                hh = 90;
                hx = attacker.rect.x - 10;
                hy = attacker.rect.y - hh + 20;
                dmgScale = 3.5f;
                kbScale  = 6.0f;
                break;
            case Status::SPECIAL_DOWN:
                hw = attacker.rect.w + 40;
                hh = 80;
                hx = attacker.rect.x - 20;
                hy = attacker.rect.y + attacker.rect.h - 20;
                dmgScale = 4.0f;
                kbScale  = 7.0f;
                if (attacker.onGround) {  // only when landing
                    shockwaves.emplace_back(attacker.rect.x + attacker.rect.w / 2,
                                            attacker.rect.y + attacker.rect.h - 16,
                                            &attacker);
                }
                break;
            default:
                return;
        };

        auto& cr = specialHitboxes.emplace_back(hx, hy, hw, hh, &attacker, 5);
        cr.damageScale = dmgScale;
        cr.kbScale     = kbScale;
    };

    trySpawnSpecialHitbox(player1);
    trySpawnSpecialHitbox(player2);

    for (auto it = shockwaves.begin(); it != shockwaves.end(); ) {
        it->update();
 
        auto tryHit = [&](Player& target) {
            if (&target == it->getOwner()) return;
            if (target.invulnerableTimer > 0) return;
            auto dir = it->checkCollision(target);
            if (!dir) return;
            int dmg = static_cast<int>(it->getOwner()->character->stats.damage * 1.5f);
            target.getHit(*dir, dmg);
        };
 
        tryHit(player1);
        tryHit(player2);
 
        if (!it->isAlive()) {
            it = shockwaves.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = specialHitboxes.begin(); it != specialHitboxes.end(); ) {
        it->update();
        if (!it->isAlive()) { it = specialHitboxes.erase(it); continue; }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer == 0) {
                int dmg = static_cast<int>(player1.character->stats.damage * it->damageScale);
                player2.getHit(player1.facing, dmg, it->kbScale);
                it = specialHitboxes.erase(it);
                continue;
            }
        } else if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer == 0) {
                int dmg = static_cast<int>(player2.character->stats.damage * it->damageScale);
                player1.getHit(player2.facing, dmg, it->kbScale);
                it = specialHitboxes.erase(it);
                continue;
            }
        }
        ++it;
    }

    // life and death (so poetical, ik)
    auto handleDeath = [&](Player& p) {
        bool voidDeath = (p.rect.y >= SH + 100);
        bool hpDead    = (p.hp <= 0);
        if (!(voidDeath || hpDead)) { return; }
        if (p.lives > 0) {
            respawn(p, voidDeath);
        } else if (p.lives == 0) {
            if (resources.gameEndSound) { Mix_PlayChannel(-1, resources.gameEndSound, 0); }
            p.lives = -1;
        }
    };
    handleDeath(player1);
    handleDeath(player2);
}

void Game::renderPlayerHud(const Player& player) const {
    int x = player == player1 ? 480 : 1315;

    // icon
    SDL_Texture* icon;
    if (player.invulnerableTimer > 0 && player.damagedTimer == 0) {
        icon = player.character->deadIcon;
    } else {
        icon = player.character->icon;
    }
    if (icon) {
        int w, h;
        SDL_QueryTexture(icon, nullptr, nullptr, &w, &h);
        SDL_Rect iconRect = { x, SH - 183 - h, w, h };
        SDL_RenderCopy(renderer, icon, nullptr, &iconRect);
    }

    // name
    Renderer::fillRect(renderer, x, 950, 150, 45, WHITE);
    Renderer::renderText(renderer, resources.font, player.name, x + 10, 955, BLACK);

    // life & lives
    if (player.lives >= 0) {
        Renderer::renderText(renderer, resources.titleFont, std::to_string(player.hp) + " hp", x, 900, WHITE);
        for (int i = 0; i < player.lives; ++i) {
            if (resources.heartImage) {
                SDL_Rect heart = { x + i * 35, 997, 30, 30 };
                SDL_RenderCopy(renderer, resources.heartImage, nullptr, &heart);
            }
        }
    } else {
        Renderer::renderText(renderer, resources.titleFont, "DEAD", x, 900, DARK_RED);
    }
    
    // charge bar
    Renderer::fillRect(renderer, x - 10, 840, 5, 57, BLACK);
    int h = static_cast<int>(player.charge * 57);
    Color c = { static_cast<int>(255.0f - player.charge * 255.0f), static_cast<int>(player.charge * 255.0f), 0 };
    Renderer::fillRect(renderer, x - 10, 840 + (57 - h), 5, h, c);
}

void Game::renderGameplay() {
    // bg
    if (resources.bgImage) {
        SDL_RenderCopy(renderer, resources.bgImage, nullptr, nullptr);
    } else {
        Renderer::fillRect(renderer, 0, 0, SW, SH, { 20, 20, 60, 255 });
    }

    // platforms
    for (auto& p : platforms) { p.draw(renderer); }

    // players and projectiles
    player1.draw(renderer, resources.smallFont);
    player2.draw(renderer, resources.smallFont);
    for (auto& pr : projectiles) { pr.draw(renderer); }

    // debug info
    if (debug) {
        // hitboxes
        for (auto& p : platforms) { p.drawHitbox(renderer); }
        player1.drawHitbox(renderer);
        player2.drawHitbox(renderer);
        for (auto& pr : projectiles)        { pr.drawHitbox(renderer); }
        for (auto& cr : meleeHitboxes)   { cr.drawHitbox(renderer); }
        for (auto& cr : specialHitboxes) { cr.drawHitbox(renderer); }
        for (auto& sw : shockwaves)          { sw.drawHitboxes(renderer); }
        
        // player statuses
        Renderer::renderText(renderer, resources.font, player1.name + ": " + player1.getStatusName(), 2, 2, BLACK);
        Renderer::renderText(renderer, resources.font, player2.name + ": " + player2.getStatusName(), 2, 32, BLACK);

        // fps
        int w, h;
        std::string text = "fps: " + std::to_string(fps);
        TTF_SizeText(resources.font, text.c_str(), &w, &h);
        Renderer::renderText(renderer, resources.font, text, SW - w - 2, h + 2, BLACK);
    }

    // player info HUD
    renderPlayerHud(player1);
    renderPlayerHud(player2);
}

void Game::handleScreenTransitions() {
    if (auto* cs = dynamic_cast<CharacterSelectionScreen*>(screen.get())) {
        if (cs->isFinished()) {
            auto result = cs->getResult();
            setupPlayers(result.char1, result.name1, result.char2, result.name2);
            player1.color = result.color1;
            player2.color = result.color2;
            player1.resetTimers();
            player2.resetTimers();
            projectiles.clear();
            meleeHitboxes.clear();
            specialHitboxes.clear();
            if (resources.music) { Mix_PlayMusic(resources.music, -1); }
            setScreen(nullptr);
        }
    }
    else if (auto* ps = dynamic_cast<PauseScreen*>(screen.get())) {
        if (ps->isFinished()) {
            switch (ps->getResult()) {
                case PauseActionResult::RESUME:
                    Mix_ResumeMusic();
                    Mix_Resume(-1);
                    setScreen(nullptr);
                    break;
                case PauseActionResult::QUIT:
                    running = false;
                    break;
                case PauseActionResult::RESTART:
                    Mix_HaltMusic();
                    Mix_HaltChannel(-1);
                    setScreen(std::make_unique<CharacterSelectionScreen>(
                        renderer, resources.titleFont, resources.font, resources.characterList(),
                        player1.name, player1.character,
                        player2.name, player2.character));
                    break;
                case PauseActionResult::CHANGE_VOLUME:
                    setScreen(std::make_unique<VolumeScreen>(
                        renderer, resources.titleFont, resources.font,
                        sfxVolume, musicVolume));
                    break;
            }
        }
    }
    else if (auto* vs = dynamic_cast<VolumeScreen*>(screen.get())) {
        if (vs->isFinished()) {
            auto vols = vs->getResult();
            sfxVolume = vols.sfx;
            musicVolume = vols.music;
            applySfxVolume(sfxVolume);
            Mix_VolumeMusic(static_cast<int>(9 * musicVolume));
            setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont));
        }
    }
}

void Game::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            break;
        }

        if (screen) {
            screen->handle(e);
        } else {
            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;
                if (key == K_PAUSE) {
                    Mix_PauseMusic();
                    setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont));
                    continue;
                }
            }
            processEvent(e);
        }
    }
}

void Game::update() {
    if (screen) {
        screen->update();
        handleScreenTransitions();
    } else {
        Mix_ResumeMusic();
        handleGameplayInput();
        updateGameplay();

        // end‑game transitions (back to character selection)
        if (player1.lives == -1 && player2.lives == -1) {
            showEndDialog("BOTH PLAYERS DIED\nwhat a skill issue");
            running = false;
        } else if (player1.lives == -1) {
            showEndDialog("GG!\n1st: " + player2.name + " (" + player2.character->stats.name + ")\n"
                          "2nd: " + player1.name + " (" + player1.character->stats.name + ")");
            Mix_HaltMusic();
            setScreen(std::make_unique<CharacterSelectionScreen>(
                renderer, resources.titleFont, resources.font, resources.characterList(),
                "player 1", &resources.BERT,
                "player 2", &resources.BERT));
        } else if (player2.lives == -1) {
            showEndDialog("GG!\n1st: " + player1.name + " (" + player1.character->stats.name + ")\n"
                          "2nd: " + player2.name + " (" + player2.character->stats.name + ")");
            Mix_HaltMusic();
            setScreen(std::make_unique<CharacterSelectionScreen>(
                renderer, resources.titleFont, resources.font, resources.characterList(),
                "player 1", &resources.BERT,
                "player 2", &resources.BERT));
        }
    }
}

void Game::render() {
    if (screen) {
        // if screen is a PauseScreen or VolumeScreen, draw the game underneath
        if (dynamic_cast<PauseScreen*>(screen.get()) || dynamic_cast<VolumeScreen*>(screen.get())) {
            renderGameplay();
        }
        screen->render(renderer, resources.font);
    } else {
        renderGameplay();
    }
    SDL_RenderPresent(renderer);

    frames++;
    Uint32 now = SDL_GetTicks();
    if (now - lastFpsUpdate >= 1000) {
        fps = frames * 1000.0f / (now - lastFpsUpdate);
        frames = 0;
        lastFpsUpdate = now;
    }
}

void Game::run() {
    setScreen(std::make_unique<CharacterSelectionScreen>(
        renderer, resources.titleFont, resources.font,
        resources.characterList(),
        "player 1", &resources.BERT,
        "player 2", &resources.BERT
    ));

    while (running) {
        beginFrame();
        processEvents();
        update();
        render();
        SDL_Delay(1000 / 60);
    }
}

void Game::setScreen(std::unique_ptr<Screen> newScreen) {
    screen = std::move(newScreen);
}

void Game::onKey(SDL_Keycode key, KeyAction action) {
    if (action != KeyAction::PRESS) { return; }

    if (key == K_QUIT) {
        running = false;
        return;
    }
    if (key == K_HITBOX) {
        debug = !debug;
        return;
    }
    if (key == K_FULLSCREEN) {
        Uint32 flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            SDL_SetWindowFullscreen(window, 0);
        } else {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        return;
    }

    // only handle gameplay inputs when no screen is active
    if (screen != nullptr) { return; }
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