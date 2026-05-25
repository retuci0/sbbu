#include "Game.h"
#include "InputHandler.h"

#include "misc/Common.h"
#include "misc/Characters.h"
#include "misc/Renderer.h"
#include "net/Packets.h"
#include "ui/screen/CharacterSelectionScreen.h"
#include "ui/screen/TitleScreen.h"
#include "ui/screen/PauseScreen.h"
#include "ui/screen/RemoteSetupScreen.h"
#include "ui/screen/VolumeScreen.h"
#include "ui/screen/WaitingScreen.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <random>

static constexpr int PADDING = 2;
static constexpr int MM_W = 200;
static constexpr int MM_H = 120;
static constexpr int MM_X = PADDING;
static constexpr int MM_Y = SH - MM_H - PADDING;


/* --- RESOURCES --- */

void Resources::destroy() {
    auto dTex   = [](SDL_Texture*& t) { if (t) { SDL_DestroyTexture(t); t = nullptr; } };
    auto dChunk = [](Mix_Chunk*&   c) { if (c) { Mix_FreeChunk(c);      c = nullptr; } };

    dTex(platformImage);
    dTex(smallPlatformImage);
    dTex(projectileImage);
    dTex(shockwaveImage);
    dTex(heartImage);
    dTex(bgImage);
    dTex(titleBgImage);

    if (titleFont) { TTF_CloseFont(titleFont); titleFont = nullptr; }
    if (font)      { TTF_CloseFont(font);           font      = nullptr; }
    if (smallFont) { TTF_CloseFont(smallFont); smallFont = nullptr; }

    dChunk(jumpSound);
    dChunk(jumpSound2);
    dChunk(deathSound);
    dChunk(projectileSound);
    dChunk(meleeSound);
    dChunk(voidDeathSound);
    dChunk(damageSound);
    dChunk(gameEndSound);

    if (specialSounds) {
        for (int i = 0; i < 4; ++i) dChunk(specialSounds[i]);
        delete[] specialSounds;
        specialSounds = nullptr;
    }

    if (music) { Mix_FreeMusic(music); music = nullptr; }
    if (titleScreenMusic) { Mix_FreeMusic(titleScreenMusic); titleScreenMusic = nullptr; }

    BERT.unload();
    BERROTA.unload();
    JORDI.unload();
    LORC.unload();
    BARCOS.unload();
    ALSEXITO.unload();
    SHASHA.unload();
    OSCAR.unload();
    FLAN.unload();
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
    throw std::runtime_error("no TTF font found: place a font.ttf in the working directory.");
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

    resources.platformImage      = loadTex("assets/images/platform/platform_big.png");
    resources.smallPlatformImage = loadTex("assets/images/platform/platform_small.png");
    resources.projectileImage    = loadTex("assets/images/projectile/projectile.png");
    resources.shockwaveImage     = loadTex("assets/images/projectile/shockwave.png");
    resources.heartImage         = loadTex("assets/images/ui/heart.png");
    resources.bgImage            = loadTex("assets/images/ui/background.png");
    resources.titleBgImage       = loadTex("assets/images/ui/titlescreen.png");

    resources.titleFont = findFont(50);
    resources.font      = findFont(30);
    resources.smallFont = findFont(21);

    resources.jumpSound         = loadChunk("assets/sound/jump.wav");
    resources.jumpSound2        = loadChunk("assets/sound/jump2.wav");
    resources.deathSound        = loadChunk("assets/sound/death.wav");
    resources.projectileSound   = loadChunk("assets/sound/projectile.wav");
    resources.meleeSound        = loadChunk("assets/sound/punch.wav");
    resources.voidDeathSound    = loadChunk("assets/sound/void_death.wav");
    resources.damageSound       = loadChunk("assets/sound/damage.wav");
    resources.gameEndSound      = loadChunk("assets/sound/game_end.wav");
    resources.music             = Mix_LoadMUS("assets/sound/music.mp3");
    resources.titleScreenMusic  = Mix_LoadMUS("assets/sound/titlescreenmusic.mp3");

    resources.specialSounds = new Mix_Chunk*[4] {
        loadChunk("assets/sound/special_static.wav"),
        loadChunk("assets/sound/special_side.wav"),
        loadChunk("assets/sound/special_up.wav"),
        loadChunk("assets/sound/special_down.wav")
    };

    if (resources.jumpSound)        Mix_VolumeChunk(resources.jumpSound,        64);
    if (resources.jumpSound2)       Mix_VolumeChunk(resources.jumpSound2,       26);
    if (resources.deathSound)       Mix_VolumeChunk(resources.deathSound,      115);
    if (resources.projectileSound)  Mix_VolumeChunk(resources.projectileSound,  26);
    if (resources.voidDeathSound)   Mix_VolumeChunk(resources.voidDeathSound,    9);
    if (resources.damageSound)      Mix_VolumeChunk(resources.damageSound,     102);
    if (resources.gameEndSound)     Mix_VolumeChunk(resources.gameEndSound,     13);
    if (resources.music)            Mix_VolumeMusic(9);
    if (resources.titleScreenMusic) Mix_VolumeMusic(9);

    resources.BERT     = loadCharacter(renderer, BERT_STATS,     "assets/images/characters/bert");
    resources.BERROTA  = loadCharacter(renderer, BERROTA_STATS,  "assets/images/characters/berrota");
    resources.JORDI    = loadCharacter(renderer, JORDI_STATS,    "assets/images/characters/jordi");
    resources.LORC     = loadCharacter(renderer, LORC_STATS,     "assets/images/characters/lorc");
    resources.BARCOS   = loadCharacter(renderer, BARCOS_STATS,   "assets/images/characters/barcos");
    resources.ALSEXITO = loadCharacter(renderer, ALSEXITO_STATS, "assets/images/characters/alsexito");
    // SHASHA, OSCAR, SASU, FLAN not yet available
}

void Game::playTitleMusic() {
    if (resources.titleScreenMusic) {
        Mix_HaltMusic();
        Mix_PlayMusic(resources.titleScreenMusic, -1);
    }
}

void Game::playGameMusic() {
    if (resources.music) {
        Mix_HaltMusic();
        Mix_PlayMusic(resources.music, -1);
    }
}


/* --- SETUP --- */

void Game::init() {
    // init sdl
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
    if (SDLNet_Init() < 0) {
        throw std::runtime_error(std::string("SDLNet_Init: ") + SDLNet_GetError());
    }

    window = SDL_CreateWindow(
        "super bert bros ultimate",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SW, SH, SDL_WINDOW_SHOWN);
    if (!window) throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
    SDL_RenderSetLogicalSize(renderer, SW, SH);

    lastFpsUpdate = SDL_GetTicks();
    loadResources();
}

void Game::setupPlayers(const Character* c1, const std::string& n1,
                        const Character* c2, const std::string& n2)
{
    platforms.clear();
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                           360, 500, 1200, 300, PlatformSize::BIG);
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                           640, 250, 200, 30, PlatformSize::SMALL);
    platforms.emplace_back(resources.platformImage, resources.smallPlatformImage,
                           1080, 250, 200, 30, PlatformSize::SMALL);

    player1.init(640, 0, c1, n1, resources.damageSound);
    player1.id = 0;

    player2.init(1080, 0, c2, n2, resources.damageSound);
    player2.id = 1;

    player1.color = {100, 149, 237, 230};
    player2.color = {255,  80,  80, 230};
}

void Game::respawn(Player& p, bool voidDeath) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> pick(0, 1);
    int spawnX = pick(rng) ? 640 : 1080;

    if (voidDeath && resources.voidDeathSound)
        Mix_PlayChannel(-1, resources.voidDeathSound, 0);
    else if (resources.deathSound)
        Mix_PlayChannel(-1, resources.deathSound, 0);

    p.hp                 = p.character->stats.health;
    p.rect.x             = spawnX;
    p.rect.y             = -2000;
    p.lives             -= 1;
    p.status             = Status::IDLE;
    p.charge             = 0.0f;
    p.dx = p.dy          = 0.0f;
    p.onGround           = false;
    p.hasAirJumped       = false;
    p.currentSpriteIndex = 0.0f;
    p.specialHitboxSpawned = false;
    p.resetTimers();
    p.invulnerableTimer  = Player::INV_DURATION;
}

void Game::applySfxVolume(float multiplier) {
    auto set = [&](Mix_Chunk* c, int base) {
        if (c) Mix_VolumeChunk(c, std::clamp(static_cast<int>(base * multiplier), 0, 128));
    };
    set(resources.deathSound,      115);
    set(resources.projectileSound,  26);
    set(resources.meleeSound,       10);
    set(resources.voidDeathSound,    9);
    set(resources.damageSound,     102);
    set(resources.gameEndSound,     13);
    if (resources.specialSounds)
        for (int i = 0; i < 4; ++i) set(resources.specialSounds[i], 64);
}

void Game::showEndDialog(const std::string& msg) {
    bool waiting  = true;
    Uint32 start  = SDL_GetTicks();
    while (waiting && SDL_GetTicks() - start < 5000) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                waiting = false;
            } 
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RETURN) {
                waiting = false;
            }
        }
        Renderer::fillRect(renderer, 0, 0, SW, SH, {0, 0, 0, 180});
        int lineY = SH / 2 - 80;
        std::string line, rest = msg;
        while (true) {
            auto nl = rest.find('\n');
            line    = (nl == std::string::npos) ? rest : rest.substr(0, nl);
            int tw, th;
            TTF_SizeText(resources.titleFont, line.c_str(), &tw, &th);
            Renderer::renderText(renderer, resources.titleFont, line, (SW - tw) / 2, lineY, WHITE);
            lineY += th + 10;
            if (nl == std::string::npos) break;
            rest = rest.substr(nl + 1);
        }
        Renderer::renderText(renderer, resources.font, "(press enter to close)", 800, lineY + 20,
                             {180, 180, 180, 255});
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}


/* --- INPUT HANDLING --- */

void Game::handleGameplayInput() {
    // player 1 - always local
    bool p1Left  = isDown(K_P1_LEFT);
    bool p1Right = isDown(K_P1_RIGHT);

    if (player1.status != Status::DAMAGED)
        player1.move((p1Left && p1Right) ? 0 : p1Left ? -1 : p1Right ? 1 : 0);

    if (isPressed(K_P1_JUMP))  player1.jump();

    if (isPressed(K_P1_SHOOT)) {
        if (player1.tryShoot(resources.projectileSound)) {
            int px = (player1.facing == Facing::LEFT) ? player1.rect.x - 20 : player1.rect.x + 20;
            projectiles.emplace_back(resources.projectileImage, px, player1.rect.y, player1.facing, &player1);
        }
    }
    if (isPressed(K_P1_MELEE)) {
        if (player1.tryMelee(resources.meleeSound)) {
            int hx = player1.rect.x + (player1.facing == Facing::RIGHT ? player1.rect.w - 64 : 0);
            meleeHitboxes.emplace_back(hx, player1.rect.y + player1.rect.h - 64, 64, 64, &player1, 5);
        }
    }
    if (isPressed(K_P1_SPECIAL)) {
        Direction dir;
        if      (isDown(K_P1_LEFT))  dir = Direction::LEFT;
        else if (isDown(K_P1_RIGHT)) dir = Direction::RIGHT;
        else if (isDown(K_P1_JUMP))  dir = Direction::UP;
        else if (isDown(K_P1_DOWN))  dir = Direction::DOWN;
        else                         dir = Direction::NONE;
        player1.trySpecial(resources.specialSounds, dir);
    }

    // player 2 - either local or remote
    bool p2Left, p2Right, p2Down;
    bool p2JumpPr, p2ShootPr, p2MeleePr, p2SpecialPr;
    bool p2JumpDn;

    if (networkMode == NetworkMode::REMOTE_HOST) {
        p2Left     = remoteIsDown(InputBit::LEFT);
        p2Right    = remoteIsDown(InputBit::RIGHT);
        p2Down     = remoteIsDown(InputBit::DOWN);
        p2JumpDn   = remoteIsDown(InputBit::JUMP);
        p2JumpPr   = remoteIsPressed(InputBit::JUMP);
        p2ShootPr  = remoteIsPressed(InputBit::SHOOT);
        p2MeleePr  = remoteIsPressed(InputBit::MELEE);
        p2SpecialPr = remoteIsPressed(InputBit::SPECIAL);
    } else {
        p2Left     = isDown(K_P2_LEFT);
        p2Right    = isDown(K_P2_RIGHT);
        p2Down     = isDown(K_P2_DOWN);
        p2JumpDn   = isDown(K_P2_JUMP);
        p2JumpPr   = isPressed(K_P2_JUMP);
        p2ShootPr  = isPressed(K_P2_SHOOT);
        p2MeleePr  = isPressed(K_P2_MELEE);
        p2SpecialPr = isPressed(K_P2_SPECIAL);
    }

    if (player2.status != Status::DAMAGED)
        player2.move((p2Left && p2Right) ? 0 : p2Left ? -1 : p2Right ? 1 : 0);

    if (p2JumpPr) player2.jump();

    if (p2ShootPr) {
        if (player2.tryShoot(resources.projectileSound)) {
            int px = (player2.facing == Facing::LEFT) ? player2.rect.x - 20 : player2.rect.x + 20;
            projectiles.emplace_back(resources.projectileImage, px, player2.rect.y, player2.facing, &player2);
        }
    }
    if (p2MeleePr) {
        if (player2.tryMelee(resources.meleeSound)) {
            int hx = player2.rect.x + (player2.facing == Facing::RIGHT ? player2.rect.w - 64 : 0);
            meleeHitboxes.emplace_back(hx, player2.rect.y + player2.rect.h - 64, 64, 64, &player2, 5);
        }
    }
    if (p2SpecialPr) {
        Direction dir;
        if      (p2Left)   dir = Direction::LEFT;
        else if (p2Right)  dir = Direction::RIGHT;
        else if (p2JumpDn) dir = Direction::UP;
        else if (p2Down)   dir = Direction::DOWN;
        else               dir = Direction::NONE;
        player2.trySpecial(resources.specialSounds, dir);
    }
}


/* --- NETWORK STUFF --- */

void Game::processNetworkPackets() {
    if (!network) return;
    std::unique_ptr<Packet> pkt;
    while (network->recv(pkt)) {
        switch (pkt->getType()) {
            case PacketType::CLIENT_INPUT: {
                auto* cip = dynamic_cast<ClientInputPacket*>(pkt.get());
                if (cip) {
                    prevRemoteInputBits = remoteInputBits;
                    remoteInputBits = cip->inputs;
                }
                break;
            }
            case PacketType::STATE_UPDATE: {
                if (networkMode == NetworkMode::REMOTE_CLIENT) {
                    auto* sup = dynamic_cast<StateUpdatePacket*>(pkt.get());
                    if (sup) netApplyStateUpdate(*sup);
                }
                break;
            }
            case PacketType::GAME_SETUP: {
                if (networkMode == NetworkMode::REMOTE_CLIENT) {
                    auto* gsp = dynamic_cast<GameSetupPacket*>(pkt.get());
                    if (gsp) {
                        pendingSetup.char1Idx = gsp->char1Idx;
                        pendingSetup.char2Idx = gsp->char2Idx;
                        strncpy(pendingSetup.name1, gsp->name1.c_str(), 31);
                        strncpy(pendingSetup.name2, gsp->name2.c_str(), 31);
                        pendingSetup.r1 = gsp->r1; pendingSetup.g1 = gsp->g1; pendingSetup.b1 = gsp->b1;
                        pendingSetup.r2 = gsp->r2; pendingSetup.g2 = gsp->g2; pendingSetup.b2 = gsp->b2;
                        hasPendingSetup = true;
                    }
                }
                break;
            }
            case PacketType::DISCONNECT:
                if (network) network->disconnect();
                break;
            default:
                break;
        }
    }
}

void Game::netSendStateUpdate() {
    StateUpdatePacket sup;
    sup.frame = netFrame;

    auto fillState = [](const Player& p) -> PlayerState {
        PlayerState ps;
        ps.x = static_cast<float>(p.rect.x);
        ps.y = static_cast<float>(p.rect.y);
        ps.dx = p.dx;
        ps.dy = p.dy;
        ps.hp = static_cast<int16_t>(p.hp);
        ps.lives = (p.lives < 0) ? 255 : static_cast<uint8_t>(p.lives);
        ps.status = static_cast<uint8_t>(p.status);
        ps.facing = static_cast<uint8_t>(p.facing);
        ps.charge = p.charge;
        ps.invulnerable = (p.invulnerableTimer > 0) ? 1 : 0;
        ps.onGround = p.onGround ? 1 : 0;
        return ps;
    };
    sup.p1 = fillState(player1);
    sup.p2 = fillState(player2);

    network->send(sup);
}

void Game::netApplyStateUpdate(const StateUpdatePacket& sup) {
    auto applyState = [](Player& p, const PlayerState& ps) {
        p.rect.x = static_cast<int>(ps.x);
        p.rect.y = static_cast<int>(ps.y);
        p.dx = ps.dx;
        p.dy = ps.dy;
        p.hp = ps.hp;
        p.lives = (ps.lives == 255) ? -1 : static_cast<int>(ps.lives);
        p.status = static_cast<Status>(ps.status);
        p.facing = static_cast<Facing>(ps.facing);
        p.charge = ps.charge;
        p.invulnerableTimer = ps.invulnerable ? Player::INV_DURATION : 0;
        p.onGround = ps.onGround != 0;
    };
    applyState(player1, sup.p1);
    applyState(player2, sup.p2);
}

void Game::netSendClientInputs() {
    uint8_t inputs = 0;
    if (isDown(K_P1_LEFT))   inputs |= InputBit::LEFT;
    if (isDown(K_P1_RIGHT))  inputs |= InputBit::RIGHT;
    if (isDown(K_P1_DOWN))   inputs |= InputBit::DOWN;
    if (isDown(K_P1_JUMP))   inputs |= InputBit::JUMP;
    if (isDown(K_P1_SHOOT))  inputs |= InputBit::SHOOT;
    if (isDown(K_P1_MELEE))  inputs |= InputBit::MELEE;
    if (isDown(K_P1_SPECIAL))inputs |= InputBit::SPECIAL;

    ClientInputPacket cip(netFrame, inputs, lastSentInputs);
    network->send(cip);
    lastSentInputs = inputs;
}


/* --- LOGIC AND SHI --- */

void Game::updateGameplay() {
    player1.update(platforms, isDown(K_P1_DOWN));
    bool p2Down = (networkMode == NetworkMode::REMOTE_HOST)
                  ? remoteIsDown(InputBit::DOWN)
                  : isDown(K_P2_DOWN);
    player2.update(platforms, p2Down);

    // projectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->move();
        if (it->rect.x >= SW || it->rect.x <= 0) { it = projectiles.erase(it); continue; }
        if (it->owner != &player1 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer > 0) { ++it; continue; }
            player1.getHit(it->direction, it->owner->character->stats.projectileDamage);
            it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
            it = projectiles.erase(it); continue;
        }
        if (it->owner != &player2 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer > 0) { ++it; continue; }
            player2.getHit(it->direction, it->owner->character->stats.projectileDamage);
            it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
            it = projectiles.erase(it); continue;
        }
        ++it;
    }
    if (projectiles.size() > MAX_PROJ) projectiles.erase(projectiles.begin());

    // melee hitboxes
    for (auto it = meleeHitboxes.begin(); it != meleeHitboxes.end(); ) {
        it->update();
        if (!it->isAlive()) { it = meleeHitboxes.erase(it); continue; }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            player2.getHit(player1.facing, it->owner->character->stats.damage);
            player1.charge = std::min(player1.charge + 0.1f, Player::MAX_CHARGE);
            it = meleeHitboxes.erase(it); continue;
        }
        if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            player1.getHit(player2.facing, it->owner->character->stats.damage);
            player2.charge = std::min(player2.charge + 0.1f, Player::MAX_CHARGE);
            it = meleeHitboxes.erase(it); continue;
        }
        ++it;
    }

    // special hitboxes
    auto trySpawnSpecialHitbox = [&](Player& attacker) {
        if (attacker.status != Status::SPECIAL_STATIC &&
            attacker.status != Status::SPECIAL_SIDE   &&
            attacker.status != Status::SPECIAL_UP     &&
            attacker.status != Status::SPECIAL_DOWN)   return;
        if (attacker.specialHitboxSpawned)             return;
        if (attacker.specialTimer > Player::SPECIAL_DURATION - 5) return;

        attacker.specialHitboxSpawned = true;
        int hx, hy, hw, hh;
        float dmgScale = 3.0f, kbScale = 5.0f;

        switch (attacker.status) {
            case Status::SPECIAL_STATIC:
                hw = 110; hh = attacker.rect.h + 20;
                hx = (attacker.facing == Facing::RIGHT) ? attacker.rect.x + attacker.rect.w - 20
                                                        : attacker.rect.x - hw + 20;
                hy = attacker.rect.y - 10;
                break;
            case Status::SPECIAL_SIDE:
                hw = 130; hh = attacker.rect.h;
                hx = (attacker.facing == Facing::RIGHT) ? attacker.rect.x + attacker.rect.w - 30
                                                        : attacker.rect.x - hw + 30;
                hy = attacker.rect.y;
                dmgScale = 2.5f; kbScale = 4.0f;
                break;
            case Status::SPECIAL_UP:
                hw = attacker.rect.w + 20; hh = 90;
                hx = attacker.rect.x - 10;
                hy = attacker.rect.y - hh + 20;
                dmgScale = 3.5f; kbScale = 6.0f;
                break;
            case Status::SPECIAL_DOWN:
                hw = attacker.rect.w + 40; hh = 80;
                hx = attacker.rect.x - 20;
                hy = attacker.rect.y + attacker.rect.h - 20;
                dmgScale = 4.0f; kbScale = 7.0f;
                if (attacker.onGround)
                    shockwaves.emplace_back(resources.shockwaveImage, attacker.rect.x + attacker.rect.w / 2,
                                           attacker.rect.y + attacker.rect.h - 16, &attacker);
                break;
            default: return;
        }
        auto& cr     = specialHitboxes.emplace_back(hx, hy, hw, hh, &attacker, 5);
        cr.damageScale = dmgScale;
        cr.kbScale     = kbScale;
    };
    trySpawnSpecialHitbox(player1);
    trySpawnSpecialHitbox(player2);

    // shockwaves
    for (auto it = shockwaves.begin(); it != shockwaves.end(); ) {
        it->update();
        auto tryHit = [&](Player& target) {
            if (&target == it->getOwner())    return;
            if (target.invulnerableTimer > 0) return;
            auto dir = it->checkCollision(target);
            if (!dir) return;
            target.getHit(*dir, static_cast<int>(it->getOwner()->character->stats.damage * 1.5f));
        };
        tryHit(player1); tryHit(player2);
        if (!it->isAlive()) it = shockwaves.erase(it); else ++it;
    }

    // special hitboxes collision
    for (auto it = specialHitboxes.begin(); it != specialHitboxes.end(); ) {
        it->update();
        if (!it->isAlive()) { it = specialHitboxes.erase(it); continue; }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer == 0) {
                player2.getHit(player1.facing,
                    static_cast<int>(player1.character->stats.damage * it->damageScale), it->kbScale);
                it = specialHitboxes.erase(it); continue;
            }
        } else if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer == 0) {
                player1.getHit(player2.facing,
                    static_cast<int>(player2.character->stats.damage * it->damageScale), it->kbScale);
                it = specialHitboxes.erase(it); continue;
            }
        }
        ++it;
    }

    // death handling
    auto handleDeath = [&](Player& p) {
        bool voidDeath = (p.rect.y >= SH + 100);
        bool hpDead    = (p.hp <= 0);
        if (!(voidDeath || hpDead)) return;
        if (p.lives > 0) {
            respawn(p, voidDeath);
        } else if (p.lives == 0) {
            if (resources.gameEndSound) Mix_PlayChannel(-1, resources.gameEndSound, 0);
            p.lives = -1;
        }
    };
    handleDeath(player1);
    handleDeath(player2);
}


/* --- RENDERING --- */

void Game::renderPlayerHud(const Player& player) const {
    int x = (player == player1) ? 480 : 1315;

    SDL_Texture* icon = (player.invulnerableTimer > 0 && player.damagedTimer == 0)
                      ? player.character->deadIcon
                      : player.character->icon;
    if (icon) {
        int w, h;
        SDL_QueryTexture(icon, nullptr, nullptr, &w, &h);
        SDL_Rect iconRect = { x, SH - 183 - h, w, h };
        SDL_RenderCopy(renderer, icon, nullptr, &iconRect);
    }

    Renderer::fillRect(renderer, x, 950, 150, 45, WHITE);
    Renderer::renderText(renderer, resources.font, player.name, x + 10, 955, BLACK);

    if (player.lives >= 0) {
        Renderer::renderText(renderer, resources.titleFont,
            std::to_string(player.hp) + " hp", x, 900, WHITE);
        for (int i = 0; i < player.lives; ++i) {
            if (resources.heartImage) {
                SDL_Rect heart = { x + i * 35, 997, 30, 30 };
                SDL_RenderCopy(renderer, resources.heartImage, nullptr, &heart);
            }
        }
    } else {
        Renderer::renderText(renderer, resources.titleFont, "DEAD", x, 900, DARK_RED);
    }

    Renderer::fillRect(renderer, x - 10, 840, 5, 57, BLACK);
    int h = static_cast<int>(player.charge * 57);
    Color c = { static_cast<int>(255.0f - player.charge * 255.0f),
                static_cast<int>(player.charge * 255.0f), 0 };
    Renderer::fillRect(renderer, x - 10, 840 + (57 - h), 5, h, c);

    // network indicator
    if (networkMode != NetworkMode::NONE && networkMode != NetworkMode::LOCAL) {
        bool isRemotePlayer = (networkMode == NetworkMode::REMOTE_HOST && player == player2)
                           || (networkMode == NetworkMode::REMOTE_CLIENT && player == player1);
        if (isRemotePlayer) {
            Renderer::renderText(renderer, resources.smallFont, "[net]", x, 870, { 130, 200, 255, 255 });
        }
    }
}

void Game::renderMinimap() const {
    // world bounds
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

    // platforms
    for (const auto& p : platforms) {
        const SDL_Rect& r = p.rect;
        minX = std::min(minX, r.x);
        maxX = std::max(maxX, r.x + r.w);
        minY = std::min(minY, r.y);
        maxY = std::max(maxY, r.y + r.h);
    }

    // players
    auto expand = [&](const Player& player) {
        minX = std::min(minX, player.rect.x);
        maxX = std::max(maxX, player.rect.x + player.rect.w);
        minY = std::min(minY, player.rect.y);
        maxY = std::max(maxY, player.rect.y + player.rect.h);
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
        const SDL_Rect& r = p.rect;
        auto [mx1, my1] = worldToMinimap(r.x, r.y);
        auto [mx2, my2] = worldToMinimap(r.x + r.w, r.y + r.h);
        int mw = static_cast<int>(mx2 - mx1);
        int mh = static_cast<int>(my2 - my1);
        if (mw > 0 && mh > 0) {
            Renderer::fillRect(renderer, static_cast<int>(mx1), static_cast<int>(my1), mw, mh, { 101, 67, 33, 200 });
        }
    }

    // draw players with their respective color
    auto drawPlayer = [&](const Player& player, Color color) {
        auto [mx, my] = worldToMinimap(player.rect.x + player.rect.w / 2.0f,
                                       player.rect.y + player.rect.h / 2.0f);
        Renderer::fillCircle(renderer, static_cast<int>(mx), static_cast<int>(my), 2, color);
    };
    drawPlayer(player1, player1.color);
    drawPlayer(player2, player2.color);
}

void Game::renderGameplay() {
    if (resources.bgImage) {
        SDL_RenderCopy(renderer, resources.bgImage, nullptr, nullptr);
    } else {
        Renderer::fillRect(renderer, 0, 0, SW, SH, {20, 20, 60, 255});
    }

    for (auto& p : platforms) p.draw(renderer);

    player1.draw(renderer, resources.smallFont);
    player2.draw(renderer, resources.smallFont);
    for (auto& pr : projectiles) pr.draw(renderer);
    for (auto& sw : shockwaves)   sw.draw(renderer);


    if (debug) {
        for (auto& p : platforms)          p.drawHitbox(renderer);
        player1.drawHitbox(renderer);
        player2.drawHitbox(renderer);
        for (auto& pr : projectiles)       pr.drawHitbox(renderer);
        for (auto& cr : meleeHitboxes)     cr.drawHitbox(renderer);
        for (auto& cr : specialHitboxes)   cr.drawHitbox(renderer);
        for (auto& sw : shockwaves)        sw.drawHitboxes(renderer);

        Renderer::renderText(renderer, resources.font,
            player1.name + ": " + player1.getStatusName(), 2, 2, BLACK);
        Renderer::renderText(renderer, resources.font,
            player2.name + ": " + player2.getStatusName(), 2, 32, BLACK);

        std::string fpsStr = "fps: " + std::to_string(fps);
        int tw, th;
        TTF_SizeText(resources.font, fpsStr.c_str(), &tw, &th);
        Renderer::renderText(renderer, resources.font, fpsStr, SW - tw - 2, th + 2, BLACK);
    }

    renderPlayerHud(player1);
    renderPlayerHud(player2);
    
    if (player1.rect.x < -player1.rect.w || player1.rect.x > SW
            || player2.rect.x < -player2.rect.w || player2.rect.x > SW
    ) {
        renderMinimap();
    }
}


/* --- SCREEN STUFF --- */

void Game::handleScreenTransitions() {
    // multiplayer mode screen (initial screen)
    if (auto* mm = dynamic_cast<TitleScreen*>(screen.get())) {
        if (!mm->isFinished()) return;
        if (mm->getResult() == MultiplayerModeResult::LOCAL) {
            networkMode = NetworkMode::LOCAL;
            setScreen(std::make_unique<CharacterSelectionScreen>(
                renderer, resources.titleFont, resources.font, resources.characterList(),
                "player 1", &resources.BERT, "player 2", &resources.BERT));
        } else {
            setScreen(std::make_unique<RemoteSetupScreen>(renderer, resources.titleFont));
        }
        return;
    }

    // remote setup (p2p)
    if (auto* rs = dynamic_cast<RemoteSetupScreen*>(screen.get())) {
        if (!rs->isFinished()) return;
        auto setupResult = rs->takeResult();
        network = std::move(setupResult.network);

        if (setupResult.role == RemoteSetupRole::HOST) {
            networkMode = NetworkMode::REMOTE_HOST;
            setScreen(std::make_unique<CharacterSelectionScreen>(
                renderer, resources.titleFont, resources.font, resources.characterList(),
                "player 1", &resources.BERT, "player 2", &resources.BERT));
        } else {
            networkMode = NetworkMode::REMOTE_CLIENT;
            setScreen(std::make_unique<WaitingScreen>(
                renderer, resources.titleFont, resources.font));
        }
        return;
    }

    // waiting screen
    if (auto* ws = dynamic_cast<WaitingScreen*>(screen.get())) {
        if (hasPendingSetup && networkMode == NetworkMode::REMOTE_CLIENT) {
            hasPendingSetup = false;
            auto charList = resources.characterList();
            uint8_t i1 = pendingSetup.char1Idx;
            uint8_t i2 = pendingSetup.char2Idx;
            setupPlayers(charList[i1], pendingSetup.name1, charList[i2], pendingSetup.name2);
            player1.color = { pendingSetup.r1, pendingSetup.g1, pendingSetup.b1, 230 };
            player2.color = { pendingSetup.r2, pendingSetup.g2, pendingSetup.b2, 230 };
            player1.resetTimers(); player2.resetTimers();
            projectiles.clear(); meleeHitboxes.clear(); specialHitboxes.clear();
            if (resources.music) Mix_PlayMusic(resources.music, -1);
            setScreen(nullptr);
            return;
        }
        // keep waiting
        return;
    }

    // character selection
    if (auto* cs = dynamic_cast<CharacterSelectionScreen*>(screen.get())) {
        if (!cs->isFinished()) return;
        auto csResult = cs->getResult();

        setupPlayers(csResult.char1, csResult.name1, csResult.char2, csResult.name2);
        player1.color = csResult.color1;
        player2.color = csResult.color2;
        player1.resetTimers();
        player2.resetTimers();
        projectiles.clear();
        meleeHitboxes.clear();
        specialHitboxes.clear();

        if (networkMode == NetworkMode::REMOTE_HOST && network && network->isConnected()) {
            auto charList = resources.characterList();
            uint8_t i1 = 0, i2 = 0;
            for (int i = 0; i < CHARACTER_NUM; ++i) {
                if (charList[i] == csResult.char1) i1 = static_cast<uint8_t>(i);
                if (charList[i] == csResult.char2) i2 = static_cast<uint8_t>(i);
            }
            GameSetupPacket gsp(i1, i2, csResult.name1, csResult.name2,
                                csResult.color1.r, csResult.color1.g, csResult.color1.b,
                                csResult.color2.r, csResult.color2.g, csResult.color2.b);
            network->send(gsp);
        }

        if (resources.music) { playGameMusic(); }
        setScreen(nullptr);
        return;
    }

    // pause screen
    if (auto* ps = dynamic_cast<PauseScreen*>(screen.get())) {
        if (!ps->isFinished()) return;
        switch (ps->getResult()) {
            case PauseActionResult::RESUME:
                Mix_ResumeMusic(); Mix_Resume(-1);
                setScreen(nullptr);
                break;
            case PauseActionResult::QUIT:
                running = false;
                break;
            case PauseActionResult::RESTART:
                Mix_HaltMusic(); Mix_HaltChannel(-1);
                if (networkMode == NetworkMode::REMOTE_HOST ||
                    networkMode == NetworkMode::REMOTE_CLIENT) {
                    if (network) network->disconnect();
                    network.reset();
                    networkMode = NetworkMode::NONE;
                    setScreen(std::make_unique<TitleScreen>(
                        renderer, resources.titleBgImage, resources.font));
                } else {
                    setScreen(std::make_unique<CharacterSelectionScreen>(
                        renderer, resources.titleFont, resources.font, resources.characterList(),
                        player1.name, player1.character,
                        player2.name, player2.character));
                }
                break;
            case PauseActionResult::CHANGE_VOLUME:
                setScreen(std::make_unique<VolumeScreen>(
                    renderer, resources.titleFont, resources.font, sfxVolume, musicVolume));
                break;
        }
        return;
    }

    //volume screen
    if (auto* vs = dynamic_cast<VolumeScreen*>(screen.get())) {
        if (!vs->isFinished()) return;
        auto vols = vs->getResult();
        sfxVolume   = vols.sfx;
        musicVolume = vols.music;
        applySfxVolume(sfxVolume);
        Mix_VolumeMusic(static_cast<int>(9 * musicVolume));
        setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont));
        return;
    }
}


/* --- MAIN LOOP --- */

void Game::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { running = false; break; }

        if (screen) {
            screen->handle(e);
        } else {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == K_PAUSE) {
                Mix_PauseMusic();
                setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont));
                continue;
            }
            processEvent(e);
        }
    }
}

void Game::update() {
    if (screen) {
        screen->update();

        // poll network even while on a screen (e.g. waiting for handshake)
        if (network) {
            network->poll();
            processNetworkPackets();
        }

        handleScreenTransitions();
        return;
    }

    // ingame
    Mix_ResumeMusic();

    if (network) {
        network->poll();
        processNetworkPackets();
    }

    if (networkMode == NetworkMode::REMOTE_CLIENT) {
        netSendClientInputs();
        // animate sprites locally
        player1.updateTimers();
        player2.updateTimers();
    } else {
        handleGameplayInput();
        updateGameplay();
        if (networkMode == NetworkMode::REMOTE_HOST && network && network->isConnected()) {
            netSendStateUpdate();
        }
    }

    netFrame++;

    // end game
    if (player1.lives == -1 && player2.lives == -1) {
        showEndDialog("BOTH PLAYERS DIED\nwhat a skill issue");
        running = false;
    } else if (player1.lives == -1) {
        showEndDialog("GG!\n1st: " + player2.name + " (" + player2.character->stats.name + ")\n"
                     "2nd: " + player1.name + " (" + player1.character->stats.name + ")");
        Mix_HaltMusic();
        if (network) { network->disconnect(); network.reset(); }
        networkMode = NetworkMode::NONE;
        setScreen(std::make_unique<TitleScreen>(renderer, resources.titleBgImage, resources.font));
    } else if (player2.lives == -1) {
        showEndDialog("GG!\n1st: " + player1.name + " (" + player1.character->stats.name + ")\n"
                     "2nd: " + player2.name + " (" + player2.character->stats.name + ")");
        Mix_HaltMusic();
        if (network) { network->disconnect(); network.reset(); }
        networkMode = NetworkMode::NONE;
        setScreen(std::make_unique<TitleScreen>(renderer, resources.titleBgImage, resources.font));
    }
}

void Game::render() {
    if (screen) {
        // if pause or volume screen, render the gameplay underneath
        if (dynamic_cast<PauseScreen*>(screen.get()) || dynamic_cast<VolumeScreen*>(screen.get()))
            renderGameplay();
        screen->render(renderer, resources.font);
    } else {
        renderGameplay();
    }
    SDL_RenderPresent(renderer);

    ++frames;
    Uint32 now = SDL_GetTicks();
    if (now - lastFpsUpdate >= 1000) {
        fps          = frames * 1000.0f / static_cast<float>(now - lastFpsUpdate);
        frames       = 0;
        lastFpsUpdate = now;
    }
}

void Game::run() {
    // start on title screen
    playTitleMusic();
    setScreen(std::make_unique<TitleScreen>(renderer, resources.titleBgImage, resources.font));

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
    if (action != KeyAction::PRESS) return;
    if (key == K_QUIT)       { running = false; return; }
    if (key == K_HITBOX)     { debug = !debug;  return; }
    if (key == K_FULLSCREEN) {
        Uint32 flags = SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window,
            (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        return;
    }
    if (screen != nullptr) return;
}


/* --- CLEANUP --- */

void Game::cleanup() {
    if (network) { network->disconnect(); network.reset(); }
    resources.destroy();
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window)   { SDL_DestroyWindow(window);     window   = nullptr; }
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}