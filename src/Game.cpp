#include "Game.h"
#include "InputHandler.h"

#include "misc/Common.h"
#include "misc/Characters.h"
#include "misc/Renderer.h"
#include "net/Packets.h"
#include "ui/screen/CharacterSelectionScreen.h"
#include "ui/screen/ControlsScreen.h"
#include "ui/screen/GameEndScreen.h"
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
#include <memory>
#include <stdexcept>
#include <random>


/* --- CONSTANTS --- */

static constexpr int PADDING = 2;
static constexpr int MM_W = 200;
static constexpr int MM_H = 120;
static constexpr int MM_X = PADDING;
static constexpr int MM_Y = SH - MM_H - PADDING;
static constexpr Uint32 REMOTE_TIMEOUT_MS = 5000;


/* --- MUSIC --- */

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
    resources.load(renderer);
    
    // load config
    options.loadFromFile();
    resources.applySfxVolume(options.sfxVolume);
    Mix_VolumeMusic(static_cast<int>(9 * options.musVolume));
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

    remoteInputBits = 0;
    prevRemoteInputBits = 0;
    lastSentInputs = 0;
    netFrame = 0;
    lastAppliedStateFrame = 0;
    hasAppliedStateFrame = false;
    hasTargetState = false;
    targetProjectiles.clear();
    pingSequence = 0;
    pendingPingSequence = 0;
    lastPingSentTicks = 0;
    pingMs = -1;
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

void Game::showEndScreen(const std::string& title, const std::string& details) {
    Mix_HaltMusic();
    if (network) {
        network->disconnect();
        network.reset();
    }
    networkMode = NetworkMode::NONE;
    setScreen(std::make_unique<GameEndScreen>(
        renderer, resources.titleFont, resources.font, title, details));
}


/* --- INPUT HANDLING --- */

void Game::handleGameplayInput() {
    // player 1 - always local
    bool p1Left  = isDown(options.keyP1Left);
    bool p1Right = isDown(options.keyP1Right);

    if (player1.status != Status::DAMAGED)
        player1.move((p1Left && p1Right) ? 0 : p1Left ? -1 : p1Right ? 1 : 0);

    if (isPressed(options.keyP1Jump))  player1.jump();

    if (isPressed(options.keyP1Shoot)) {
        if (player1.tryShoot(resources.projectileSound)) {
            int px = (player1.facing == Facing::LEFT) ? player1.rect.x - 20 : player1.rect.x + 20;
            projectiles.emplace_back(resources.projectileImage, px, player1.rect.y, player1.facing, &player1);
        }
    }
    if (isPressed(options.keyP1Melee)) {
        if (player1.tryMelee(resources.meleeSound)) {
            const int hw = 86;
            const int hh = 76;
            int hx = (player1.facing == Facing::RIGHT)
                   ? player1.rect.x + player1.rect.w - 36
                   : player1.rect.x - hw + 36;
            int hy = player1.rect.y + (player1.rect.h - hh) / 2;
            meleeHitboxes.emplace_back(hx, hy, hw, hh, &player1, 6);
        }
    }
    if (isPressed(options.keyP1Special)) {
        Direction dir;
        if      (isDown(options.keyP1Left))  dir = Direction::LEFT;
        else if (isDown(options.keyP1Right)) dir = Direction::RIGHT;
        else if (isDown(options.keyP1Jump))  dir = Direction::UP;
        else if (isDown(options.keyP1Down))  dir = Direction::DOWN;
        else                                      dir = Direction::NONE;
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
        p2Left     = isDown(options.keyP2Left);
        p2Right    = isDown(options.keyP2Right);
        p2Down     = isDown(options.keyP2Down);
        p2JumpDn   = isDown(options.keyP2Jump);
        p2JumpPr   = isPressed(options.keyP2Jump);
        p2ShootPr  = isPressed(options.keyP2Shoot);
        p2MeleePr  = isPressed(options.keyP2Melee);
        p2SpecialPr = isPressed(options.keyP2Special);
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
            const int hw = 86;
            const int hh = 76;
            int hx = (player2.facing == Facing::RIGHT)
                   ? player2.rect.x + player2.rect.w - 36
                   : player2.rect.x - hw + 36;
            int hy = player2.rect.y + (player2.rect.h - hh) / 2;
            meleeHitboxes.emplace_back(hx, hy, hw, hh, &player2, 6);
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
                    prevRemoteInputBits = cip->lastInputs;
                    remoteInputBits = cip->inputs;
                }
                break;
            }
            case PacketType::STATE_UPDATE: {
                if (networkMode == NetworkMode::REMOTE_CLIENT && !screen) {
                    auto* sup = dynamic_cast<StateUpdatePacket*>(pkt.get());
                    if (sup) netApplyStateUpdate(*sup);
                }
                break;
            }
            case PacketType::GAME_SETUP: {
                if (networkMode == NetworkMode::REMOTE_CLIENT) {
                    auto* gsp = dynamic_cast<GameSetupPacket*>(pkt.get());
                    if (gsp) {
                        if (gsp->char1Idx >= CHARACTER_NUM || gsp->char2Idx >= CHARACTER_NUM) {
                            break;
                        }
                        pendingSetup.char1Idx = gsp->char1Idx;
                        pendingSetup.char2Idx = gsp->char2Idx;
                        pendingSetup.name1 = gsp->name1;
                        pendingSetup.name2 = gsp->name2;
                        pendingSetup.r1 = gsp->r1; pendingSetup.g1 = gsp->g1; pendingSetup.b1 = gsp->b1;
                        pendingSetup.r2 = gsp->r2; pendingSetup.g2 = gsp->g2; pendingSetup.b2 = gsp->b2;
                        hasPendingSetup = true;
                    }
                }
                break;
            }
            case PacketType::DISCONNECT:
                if (network) network->disconnect(false);
                if (!screen && player1.lives >= 0 && player2.lives >= 0) {
                    if (networkMode == NetworkMode::REMOTE_HOST) {
                        player2.lives = -1;
                    } else if (networkMode == NetworkMode::REMOTE_CLIENT) {
                        player1.lives = -1;
                    }
                }
                break;
            case PacketType::PING: {
                auto* ping = dynamic_cast<PingPacket*>(pkt.get());
                if (ping && network && network->isConnected()) {
                    PongPacket pong(ping->sequence, ping->sentTicks);
                    network->send(pong);
                }
                break;
            }
            case PacketType::PONG: {
                auto* pong = dynamic_cast<PongPacket*>(pkt.get());
                if (pong && pong->sequence == pendingPingSequence) {
                    pingMs = static_cast<int>(SDL_GetTicks() - pong->sentTicks);
                    pendingPingSequence = 0;
                }
                break;
            }
            default:
                break;
        }
    }
}

void Game::netUpdatePing() {
    if (!network || !network->isConnected()) return;

    Uint32 now = SDL_GetTicks();
    if (now - lastPingSentTicks < 1000) return;

    lastPingSentTicks = now;
    pendingPingSequence = ++pingSequence;
    PingPacket ping(pendingPingSequence, now);
    network->send(ping);
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
    sup.projectiles.reserve(projectiles.size());
    for (const auto& projectile : projectiles) {
        ProjectileState ps;
        ps.x = static_cast<float>(projectile.rect.x);
        ps.y = static_cast<float>(projectile.rect.y);
        ps.velocity = projectile.velocity;
        ps.facing = static_cast<uint8_t>(projectile.direction);
        ps.ownerId = projectile.owner ? static_cast<uint8_t>(projectile.owner->id) : 0;
        ps.parryFreezeTimer = static_cast<uint8_t>(std::clamp(projectile.parryFreezeTimer, 0, 255));
        ps.parryFlashTimer = static_cast<uint8_t>(std::clamp(projectile.parryFlashTimer, 0, 255));
        sup.projectiles.push_back(ps);
    }

    network->send(sup);
}

void Game::netApplyStateUpdate(const StateUpdatePacket& sup) {
    if (hasAppliedStateFrame && sup.frame <= lastAppliedStateFrame) return;
    lastAppliedStateFrame = sup.frame;
    hasAppliedStateFrame = true;
    targetPlayer1State = sup.p1;
    targetPlayer2State = sup.p2;
    targetProjectiles = sup.projectiles;

    auto applyState = [](Player& p, const PlayerState& ps, bool snapPosition) {
        if (snapPosition) {
            p.rect.x = static_cast<int>(ps.x);
            p.rect.y = static_cast<int>(ps.y);
        }
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
    applyState(player1, sup.p1, !hasTargetState);
    applyState(player2, sup.p2, !hasTargetState);

    if (!hasTargetState || projectiles.size() != sup.projectiles.size()) {
        projectiles.clear();
        projectiles.reserve(sup.projectiles.size());
        for (const auto& ps : sup.projectiles) {
            Player* owner = (ps.ownerId == 1) ? &player2 : &player1;
            projectiles.emplace_back(resources.projectileImage,
                                     static_cast<int>(ps.x),
                                     static_cast<int>(ps.y),
                                     static_cast<Facing>(ps.facing),
                                     owner);
            projectiles.back().velocity = ps.velocity;
            projectiles.back().parryFreezeTimer = ps.parryFreezeTimer;
            projectiles.back().parryFlashTimer = ps.parryFlashTimer;
        }
    }
    hasTargetState = true;
}

void Game::netInterpolateRemoteState() {
    if (!hasTargetState) return;

    auto approach = [](int current, float target) {
        float next = static_cast<float>(current) + (target - static_cast<float>(current)) * 0.35f;
        return static_cast<int>(next);
    };

    player1.rect.x = approach(player1.rect.x, targetPlayer1State.x);
    player1.rect.y = approach(player1.rect.y, targetPlayer1State.y);
    player2.rect.x = approach(player2.rect.x, targetPlayer2State.x);
    player2.rect.y = approach(player2.rect.y, targetPlayer2State.y);

    if (projectiles.size() != targetProjectiles.size()) return;
    for (size_t i = 0; i < projectiles.size(); ++i) {
        const auto& target = targetProjectiles[i];
        projectiles[i].rect.x = approach(projectiles[i].rect.x, target.x);
        projectiles[i].rect.y = approach(projectiles[i].rect.y, target.y);
        projectiles[i].velocity = target.velocity;
        projectiles[i].direction = static_cast<Facing>(target.facing);
        projectiles[i].owner = (target.ownerId == 1) ? &player2 : &player1;
        projectiles[i].parryFreezeTimer = target.parryFreezeTimer;
        projectiles[i].parryFlashTimer = target.parryFlashTimer;
    }
}

void Game::netSendClientInputs() {
    uint8_t inputs = 0;
    if (isDown(options.keyP1Left))   inputs |= InputBit::LEFT;
    if (isDown(options.keyP1Right))  inputs |= InputBit::RIGHT;
    if (isDown(options.keyP1Down))   inputs |= InputBit::DOWN;
    if (isDown(options.keyP1Jump))   inputs |= InputBit::JUMP;
    if (isDown(options.keyP1Shoot))  inputs |= InputBit::SHOOT;
    if (isDown(options.keyP1Melee))  inputs |= InputBit::MELEE;
    if (isDown(options.keyP1Special))inputs |= InputBit::SPECIAL;

    ClientInputPacket cip(netFrame, inputs, lastSentInputs);
    network->send(cip);
    lastSentInputs = inputs;
}


/* --- LOGIC AND SHI --- */

void Game::updateGameplay() {
    player1.update(platforms, isDown(options.keyP1Down));
    bool p2Down = (networkMode == NetworkMode::REMOTE_HOST)
                  ? remoteIsDown(InputBit::DOWN)
                  : isDown(options.keyP2Down);
    player2.update(platforms, p2Down);

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
                                           attacker.rect.y + attacker.rect.h - 32, &attacker);
                break;
            default: 
                return;
        }
        auto& cr = specialHitboxes.emplace_back(hx, hy, hw, hh, &attacker, 5);
        cr.damageScale = dmgScale;
        cr.kbScale     = kbScale;
    };
    trySpawnSpecialHitbox(player1);
    trySpawnSpecialHitbox(player2);

    auto tryParryProjectile = [&](Projectile& projectile) {
        auto tryHitbox = [&](const CollisionRect& hitbox) {
            if (!hitbox.owner || projectile.owner == hitbox.owner) return false;
            if (projectile.parryFreezeTimer > 0) return false;
            if (!SDL_HasIntersection(&projectile.rect, &hitbox.rect)) return false;

            projectile.parry(hitbox.owner);
            hitbox.owner->charge = std::min(hitbox.owner->charge + 0.2f, Player::MAX_CHARGE);
            if (resources.parrySound) Mix_PlayChannel(-1, resources.parrySound, 0);
            return true;
        };

        for (const auto& hitbox : meleeHitboxes) {
            if (tryHitbox(hitbox)) return true;
        }
        for (const auto& hitbox : specialHitboxes) {
            if (tryHitbox(hitbox)) return true;
        }
        return false;
    };

    // projectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        if (tryParryProjectile(*it)) { ++it; continue; }
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


    if (options.debug) {
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
        Renderer::renderText(renderer, resources.font, fpsStr, SW - tw - 2, 2, BLACK);

        if (networkMode == NetworkMode::REMOTE_HOST || networkMode == NetworkMode::REMOTE_CLIENT) {
            std::string pingStr = "ping: ";
            pingStr += (pingMs >= 0) ? std::to_string(pingMs) + " ms" : "? ms";
            TTF_SizeText(resources.font, pingStr.c_str(), &tw, &th);
            Renderer::renderText(renderer, resources.font, pingStr, SW - tw - 2, 34, BLACK);
        }
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
            setScreen(std::make_unique<RemoteSetupScreen>(renderer, resources.titleFont, resources.font));
        }
        return;
    }

    // remote setup (p2p)
    if (auto* rs = dynamic_cast<RemoteSetupScreen*>(screen.get())) {
        if (!rs->isFinished()) return;
        if (rs->shouldGoBack()) {
            setScreen(std::make_unique<TitleScreen>(renderer, resources.titleBgImage, resources.font));
            return;
        }
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
            if (!charList[i1] || !charList[i2] || !charList[i1]->loaded || !charList[i2]->loaded) {
                return;
            }
            setupPlayers(charList[i1], pendingSetup.name1, charList[i2], pendingSetup.name2);
            player1.color = { pendingSetup.r1, pendingSetup.g1, pendingSetup.b1, 230 };
            player2.color = { pendingSetup.r2, pendingSetup.g2, pendingSetup.b2, 230 };
            player1.resetTimers(); player2.resetTimers();
            projectiles.clear(); meleeHitboxes.clear(); specialHitboxes.clear();
            if (resources.music) playGameMusic();
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

        if (networkMode == NetworkMode::REMOTE_HOST && (!network || !network->isConnected())) {
            return;
        }
        if (!csResult.char1 || !csResult.char2 || !csResult.char1->loaded || !csResult.char2->loaded) {
            return;
        }

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
                    renderer, resources.titleFont, resources.font, options.sfxVolume, options.musVolume));
                break;
            case PauseActionResult::CHANGE_CONTROLS:
                setScreen(std::make_unique<ControlsScreen>(
                    renderer, resources.titleFont, resources.font, options));
                break;
        }
        return;
    }

    //volume screen
    if (auto* vs = dynamic_cast<VolumeScreen*>(screen.get())) {
        if (!vs->isFinished()) return;
        auto vols = vs->getResult();
        options.sfxVolume   = vols.sfx;
        options.musVolume = vols.music;
        resources.applySfxVolume(options.sfxVolume);
        Mix_VolumeMusic(static_cast<int>(9 * options.musVolume));
        setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont, resources.font));
        return;
    }

    if (auto* cs = dynamic_cast<ControlsScreen*>(screen.get())) {
        if (!cs->isFinished()) return;
        options.saveToFile();
        setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont, resources.font));
        return;
    }

    if (auto* ge = dynamic_cast<GameEndScreen*>(screen.get())) {
        if (!ge->isFinished()) return;
        if (ge->getResult() == GameEndActionResult::QUIT) {
            running = false;
            return;
        }
        playTitleMusic();
        setScreen(std::make_unique<TitleScreen>(renderer, resources.titleBgImage, resources.font));
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
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == options.keyPause) {
                Mix_PauseMusic();
                setScreen(std::make_unique<PauseScreen>(renderer, SW, SH, resources.titleFont, resources.font));
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
            netUpdatePing();
        }

        handleScreenTransitions();
        return;
    }

    // ingame
    Mix_ResumeMusic();

    if (network) {
        network->poll();
        processNetworkPackets();
        netUpdatePing();
        if (network->isConnected()
                && (networkMode == NetworkMode::REMOTE_HOST || networkMode == NetworkMode::REMOTE_CLIENT)
                && SDL_GetTicks() - network->getLastReceiveTicks() > REMOTE_TIMEOUT_MS
                && player1.lives >= 0 && player2.lives >= 0) {
            network->disconnect(false);
            if (networkMode == NetworkMode::REMOTE_HOST) {
                player2.lives = -1;
            } else {
                player1.lives = -1;
            }
        }
    }

    if (networkMode == NetworkMode::REMOTE_CLIENT) {
        netSendClientInputs();
        netInterpolateRemoteState();
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
        showEndScreen("both players died", "what a skill issue");
    } else if (player1.lives == -1) {
        showEndScreen("gg!", "1st: " + player2.name + " (" + player2.character->stats.name + ")   "
                            "2nd: " + player1.name + " (" + player1.character->stats.name + ")");
    } else if (player2.lives == -1) {
        showEndScreen("gg!", "1st: " + player1.name + " (" + player1.character->stats.name + ")   "
                            "2nd: " + player2.name + " (" + player2.character->stats.name + ")");
    }
}

void Game::render() {
    if (screen) {
        // if pause or volume screen, render the gameplay underneath
        if (dynamic_cast<PauseScreen*>(screen.get()) || dynamic_cast<VolumeScreen*>(screen.get()))
            renderGameplay();
        screen->render(renderer);
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
    if (key == options.keyQuit)       { running = false; return; }
    if (key == options.keyDebug)      { options.debug = !options.debug;  return; }
    if (key == options.keyFullscreen) {
        Uint32 flags = SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window,
            (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        return;
    }
    if (screen != nullptr) return;
}


/* --- CLEANUP --- */

void Game::cleanup() {
    options.saveToFile();
    if (network) { network->disconnect(); network.reset(); }
    resources.destroy();
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window)   { SDL_DestroyWindow(window);     window   = nullptr; }
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
