#include "core/Game.h"

#include "ui/screen/WaitingScreen.h"


/////////////////////////////////////////
/*               NETWORK               */
/////////////////////////////////////////

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
                if (networkMode == NetworkMode::REMOTE_CLIENT && !screens) {
                    auto* sup = dynamic_cast<StateUpdatePacket*>(pkt.get());
                    if (sup) netApplyStateUpdate(*sup);
                }
                break;
            }
            case PacketType::GAME_SETUP: {
                if (networkMode == NetworkMode::REMOTE_CLIENT) {
                    auto* gsp = dynamic_cast<GameSetupPacket*>(pkt.get());
                    if (gsp) {
                        // only apply setup if in the waiting screen
                        if (screens.currentAs<WaitingScreen>()) {
                            if (gsp->char1Idx >= CHARACTER_NUM || gsp->char2Idx >= CHARACTER_NUM)
                                break;
                            pendingSetup.char1Idx = gsp->char1Idx;
                            pendingSetup.char2Idx = gsp->char2Idx;
                            pendingSetup.stageIdx = gsp->stageIdx;
                            pendingSetup.name1 = gsp->name1;
                            pendingSetup.name2 = gsp->name2;
                            pendingSetup.r1 = gsp->r1; pendingSetup.g1 = gsp->g1; pendingSetup.b1 = gsp->b1;
                            pendingSetup.r2 = gsp->r2; pendingSetup.g2 = gsp->g2; pendingSetup.b2 = gsp->b2;
                            hasPendingSetup = true;
                        }
                    }
                }
                break;
            }
            case PacketType::DISCONNECT:
                if (network) network->disconnect(false);
                if (!screens && player1.lives >= 0 && player2.lives >= 0) {
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
                    ping = static_cast<int>(SDL_GetTicks() - pong->sentTicks);
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
        ps.parryFreezeTimer = static_cast<uint8_t>(std::clamp(projectile.parryFreezeTimer, 0.0f, 255.0f));
        ps.parryFlashTimer = static_cast<uint8_t>(std::clamp(projectile.parryFlashTimer, 0.0f, 255.0f));
        sup.projectiles.push_back(ps);
    }

    network->send(sup);
}

void Game::netApplyStateUpdate(const StateUpdatePacket& sup) {
    if (hasAppliedStateFrame && sup.frame <= lastAppliedStateFrame) return;
    lastAppliedStateFrame = sup.frame;
    hasAppliedStateFrame = true;

    // for interpolation
    player1.prevRect = player1.rect;
    player2.prevRect = player2.rect;

    // authorative state for player1
    player1.rect.x = static_cast<int>(sup.p1.x);
    player1.rect.y = static_cast<int>(sup.p1.y);
    player1.dx = sup.p1.dx;
    player1.dy = sup.p1.dy;
    player1.hp = sup.p1.hp;
    player1.lives = (sup.p1.lives == 255) ? -1 : static_cast<int>(sup.p1.lives);
    player1.status = static_cast<Status>(sup.p1.status);
    player1.facing = static_cast<Facing>(sup.p1.facing);
    player1.charge = sup.p1.charge;
    player1.invulnerableTimer = sup.p1.invulnerable ? Player::INV_DURATION : 0;
    player1.onGround = sup.p1.onGround != 0;

    // authorative state for player2
    player2.rect.x = static_cast<int>(sup.p2.x);
    player2.rect.y = static_cast<int>(sup.p2.y);
    player2.dx = sup.p2.dx;
    player2.dy = sup.p2.dy;
    player2.hp = sup.p2.hp;
    player2.lives = (sup.p2.lives == 255) ? -1 : static_cast<int>(sup.p2.lives);
    player2.status = static_cast<Status>(sup.p2.status);
    player2.facing = static_cast<Facing>(sup.p2.facing);
    player2.charge = sup.p2.charge;
    player2.invulnerableTimer = sup.p2.invulnerable ? Player::INV_DURATION : 0;
    player2.onGround = sup.p2.onGround != 0;

    // handle projs
    if (projectiles.size() != sup.projectiles.size()) {
        projectiles.clear();
        projectiles.reserve(sup.projectiles.size());
        // rebuild if count changed
        for (const auto& ps : sup.projectiles) {
            Player* owner = (ps.ownerId == 1) ? &player2 : &player1;
            projectiles.emplace_back( static_cast<int>(ps.x), static_cast<int>(ps.y),
                                     static_cast<Facing>(ps.facing), owner);
            projectiles.back().velocity = ps.velocity;
            projectiles.back().parryFreezeTimer = ps.parryFreezeTimer;
            projectiles.back().parryFlashTimer = ps.parryFlashTimer;
            projectiles.back().prevRect = projectiles.back().rect;  // init prev
        }
    } else {
        // update existing
        for (size_t i = 0; i < projectiles.size(); ++i) {
            projectiles[i].prevRect = projectiles[i].rect;
            projectiles[i].rect.x = static_cast<int>(sup.projectiles[i].x);
            projectiles[i].rect.y = static_cast<int>(sup.projectiles[i].y);
            projectiles[i].velocity = sup.projectiles[i].velocity;
            projectiles[i].direction = static_cast<Facing>(sup.projectiles[i].facing);
            projectiles[i].owner = (sup.projectiles[i].ownerId == 1) ? &player2 : &player1;
            projectiles[i].parryFreezeTimer = sup.projectiles[i].parryFreezeTimer;
            projectiles[i].parryFlashTimer = sup.projectiles[i].parryFlashTimer;
        }
    }

    hasTargetState = true;
}

void Game::netSendClientInputs() {
    uint16_t inputs = 0;
    // keyboard
    if (isDown(options.keyP1Left))    inputs |= InputBit::LEFT;
    if (isDown(options.keyP1Right))   inputs |= InputBit::RIGHT;
    if (isDown(options.keyP1Down))    inputs |= InputBit::DOWN;
    if (isDown(options.keyP1Jump))    inputs |= InputBit::JUMP;
    if (isDown(options.keyP1Shoot))   inputs |= InputBit::SHOOT;
    if (isDown(options.keyP1Melee))   inputs |= InputBit::MELEE;
    if (isDown(options.keyP1Special)) inputs |= InputBit::SPECIAL;
    if (isDown(options.keyP1Shield))  inputs |= InputBit::SHIELD;
    if (isDown(options.keyP1Dash))    inputs |= InputBit::DASH;
    // controller 0
    float axisX = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTX, 0);
    float axisY = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 0);
    if (axisX < -0.2f)  inputs |= InputBit::LEFT;
    if (axisX >  0.2f)  inputs |= InputBit::RIGHT;
    if (axisY >  0.5f)  inputs |= InputBit::DOWN;
    if (isDown(SDL_CONTROLLER_BUTTON_A,           0)) inputs |= InputBit::JUMP;
    if (isDown(SDL_CONTROLLER_BUTTON_B,           0)) inputs |= InputBit::SHOOT;
    if (isDown(SDL_CONTROLLER_BUTTON_X,           0)) inputs |= InputBit::MELEE;
    if (isDown(SDL_CONTROLLER_BUTTON_Y,           0)) inputs |= InputBit::SPECIAL;
    if (isDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,0)) inputs |= InputBit::SHIELD;
    if (isDown(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,0)) inputs |= InputBit::DASH;

    ClientInputPacket cip(netFrame, inputs, lastSentInputs);
    network->send(cip);
    lastSentInputs = inputs;
}
