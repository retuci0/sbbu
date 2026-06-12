#include "core/Game.h"

#include "entity/Grapple.h"
#include "entity/Items.h"

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
                if (!screens && player1->lives >= 0 && player2->lives >= 0) {
                    if (networkMode == NetworkMode::REMOTE_HOST) {
                        player2->lives = -1;
                    } else if (networkMode == NetworkMode::REMOTE_CLIENT) {
                        player1->lives = -1;
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

    // players
    auto fillState = [](const Player* p) -> PlayerNetState {
        PlayerNetState ps;
        ps.x = static_cast<float>(p->rect.x);
        ps.y = static_cast<float>(p->rect.y);
        ps.dx = p->dx;
        ps.dy = p->dy;
        ps.hp = static_cast<int16_t>(p->hp);
        ps.lives = (p->lives < 0) ? 255 : static_cast<uint8_t>(p->lives);
        ps.status = static_cast<uint8_t>(p->status);
        ps.facing = static_cast<uint8_t>(p->facing);
        ps.charge = p->charge;
        ps.invulnerable = (p->invulnerableTimer > 0) ? 1 : 0;
        ps.onGround = p->onGround ? 1 : 0;
        return ps;
    };
    sup.p1 = fillState(player1);
    sup.p2 = fillState(player2);

    // grapple
    auto fillGrapple = [&](const Player* p, const std::vector<Projectile*>& projs) -> GrappleNetState {
        GrappleNetState g;
        if (!p->grapple) return g;
        const Grapple* gr = p->grapple;
        g.active = 1;
        g.x      = static_cast<float>(gr->rect.x);
        g.y      = static_cast<float>(gr->rect.y);
        g.dx     = gr->dx;
        g.dy     = gr->dy;
        g.state  = static_cast<uint8_t>(gr->state);
        g.playerDx0           = gr->playerDx0;
        g.playerDy0           = gr->playerDy0;
        g.velocitySnapshotted = gr->velocitySnapshotted ? 1 : 0;

        if (gr->targetPlayer) {
            g.targetKind  = 1;
            g.targetIndex = static_cast<uint8_t>(gr->targetPlayer->id);
        } else if (gr->targetProjectile) {
            g.targetKind = 2;
            // find index in projectiles list
            for (size_t i = 0; i < projs.size(); ++i) {
                if (projs[i] == gr->targetProjectile) {
                    g.targetIndex = static_cast<uint8_t>(std::min<size_t>(i, 254));
                    break;
                }
            }
        } else if (gr->targetPoint) {
            g.targetKind = 3;
            for (size_t i = 0; i < grapplePoints.size(); ++i) {
                if (grapplePoints[i] == gr->targetPoint) {
                    g.targetIndex = static_cast<uint8_t>(std::min<size_t>(i, 254));
                    break;
                }
            }
        } else if (gr->targetItem) {
            g.targetKind = 4;
            for (size_t i = 0; i < items.size(); ++i) {
                if (items[i] == gr->targetItem) {
                    g.targetIndex = static_cast<uint8_t>(std::min<size_t>(i, 254));
                    break;
                }
            }
        }
        return g;
    };
    sup.grapple1 = fillGrapple(player1, projectiles);
    sup.grapple2 = fillGrapple(player2, projectiles);

    // projectiles
    sup.projectiles.reserve(projectiles.size());
    for (Projectile* proj : projectiles) {
        ProjectileNetState ps;
        ps.x = static_cast<float>(proj->rect.x);
        ps.y = static_cast<float>(proj->rect.y);
        ps.velocity = proj->velocity;
        ps.facing = static_cast<uint8_t>(proj->facing);
        ps.ownerId = proj->owner ? static_cast<uint8_t>(proj->owner->id) : 0;
        ps.parryFreezeTimer = static_cast<uint8_t>(std::clamp(proj->parryFreezeTimer, 0.0f, 255.0f));
        ps.parryFlashTimer = static_cast<uint8_t>(std::clamp(proj->parryFlashTimer, 0.0f, 255.0f));
        sup.projectiles.push_back(ps);
    }

    // items
    for (Item* item : items) {
        ItemNetState ins;
        ins.x           = static_cast<float>(item->rect.x);
        ins.y           = static_cast<float>(item->rect.y);
        ins.typeIdx     = item->getTypeIdx();
        ins.alive       = item->isAlive() ? 1 : 0;
        ins.active      = item->isActive() ? 1 : 0;
        ins.effectTimer = item->effectTimer;
        ins.hp          = static_cast<float>(item->hp);
        sup.items.push_back(ins);
    }
 
    // platforms
    sup.platformActive.reserve(platforms.size());
    for (Platform* plat : platforms) {
        sup.platformActive.push_back(plat->active ? 1 : 0);
    }
 
    // countdown
    sup.countdownTimer  = countdownTimer;
    sup.countdownActive = countdownActive ? 1 : 0;

    network->send(sup);
}

void Game::netApplyStateUpdate(const StateUpdatePacket& sup) {
    if (hasAppliedStateFrame && sup.frame <= lastAppliedStateFrame) return;
    lastAppliedStateFrame = sup.frame;
    hasAppliedStateFrame = true;
 
    // interpolation
    player1->prevRect = player1->rect;
    player2->prevRect = player2->rect;
 
    // authoritative state for player1
    player1->rect.x = static_cast<int>(sup.p1.x);
    player1->rect.y = static_cast<int>(sup.p1.y);
    player1->dx = sup.p1.dx;
    player1->dy = sup.p1.dy;
    player1->hp = sup.p1.hp;
    player1->lives = (sup.p1.lives == 255) ? -1 : static_cast<int>(sup.p1.lives);
    player1->status = static_cast<Status>(sup.p1.status);
    player1->facing = static_cast<Facing>(sup.p1.facing);
    player1->charge = sup.p1.charge;
    player1->invulnerableTimer = sup.p1.invulnerable ? Player::INV_DURATION : 0;
    player1->onGround = sup.p1.onGround != 0;
 
    // authoritative state for player2
    player2->rect.x = static_cast<int>(sup.p2.x);
    player2->rect.y = static_cast<int>(sup.p2.y);
    player2->dx = sup.p2.dx;
    player2->dy = sup.p2.dy;
    player2->hp = sup.p2.hp;
    player2->lives = (sup.p2.lives == 255) ? -1 : static_cast<int>(sup.p2.lives);
    player2->status = static_cast<Status>(sup.p2.status);
    player2->facing = static_cast<Facing>(sup.p2.facing);
    player2->charge = sup.p2.charge;
    player2->invulnerableTimer = sup.p2.invulnerable ? Player::INV_DURATION : 0;
    player2->onGround = sup.p2.onGround != 0;
 
    // handle projectiles
    for (Projectile* proj : projectiles) {
        destroyEntity(proj);
    }
    projectiles.clear();
 
    // rebuild from network state
    projectiles.reserve(sup.projectiles.size());
    for (const auto& ps : sup.projectiles) {
        Player* owner = (ps.ownerId == 1) ? player2 : player1;
        Projectile* proj = spawnEntity<Projectile>(
            static_cast<int>(ps.x), static_cast<int>(ps.y),
            static_cast<Facing>(ps.facing), owner
        );
        proj->velocity = ps.velocity;
        proj->parryFreezeTimer = ps.parryFreezeTimer;
        proj->parryFlashTimer = ps.parryFlashTimer;
        proj->prevRect = proj->rect;  // init prev for interpolation
        projectiles.push_back(proj);
    }
 
    // apply grapple state for both players
    auto applyGrapple = [&](Player* p, const GrappleNetState& g) {
        if (!g.active) {
            // host says no grapple: tear down any local one
            if (p->grapple) p->ungrapple();
            return;
        }
 
        // create or reuse the grapple object
        if (!p->grapple) {
            p->grapple = new Grapple(*p,
                static_cast<int>(g.x), static_cast<int>(g.y),
                g.dx, g.dy);
        }
        Grapple* gr = p->grapple;
 
        gr->prevRect = gr->rect;
        gr->rect.x   = static_cast<int>(g.x);
        gr->rect.y   = static_cast<int>(g.y);
        gr->dx       = g.dx;
        gr->dy       = g.dy;
        gr->state    = static_cast<GrappleState>(g.state);
        gr->playerDx0           = g.playerDx0;
        gr->playerDy0           = g.playerDy0;
        gr->velocitySnapshotted = g.velocitySnapshotted != 0;
 
        // restore target pointers from indices
        gr->targetPlayer     = nullptr;
        gr->targetProjectile = nullptr;
        gr->targetPoint      = nullptr;
        gr->targetItem       = nullptr;
 
        switch (g.targetKind) {
            case 1: // player
                gr->targetPlayer = (g.targetIndex == static_cast<uint8_t>(player1->id)) ? player1 : player2;
                break;
            case 2: // projectile
                if (g.targetIndex < projectiles.size())
                    gr->targetProjectile = projectiles[g.targetIndex];
                break;
            case 3: // grapple point
                if (g.targetIndex < grapplePoints.size())
                    gr->targetPoint = grapplePoints[g.targetIndex];
                break;
            case 4: // item
                if (g.targetIndex < items.size())
                    gr->targetItem = items[g.targetIndex];
                break;
            default:
                break;
        }
    };
    applyGrapple(player1, sup.grapple1);
    applyGrapple(player2, sup.grapple2);
 
    // sync items
    while (items.size() > sup.items.size()) {
        Item* last = items.back();
        items.pop_back();
        destroyEntity(last);
    }
    for (size_t i = 0; i < sup.items.size(); ++i) {
        const ItemNetState& ins = sup.items[i];
        Item* item = nullptr;
        if (i >= items.size()) {
            std::unique_ptr<Item> newItem;
            switch (ins.typeIdx) {
                case 0:  newItem = std::make_unique<MushroomItem>(static_cast<int>(ins.x), static_cast<int>(ins.y));   break;
                case 1:  newItem = std::make_unique<ShitItem>(static_cast<int>(ins.x), static_cast<int>(ins.y));       break;
                case 2:  newItem = std::make_unique<CocaineItem>(static_cast<int>(ins.x), static_cast<int>(ins.y));    break;
                case 3:  newItem = std::make_unique<SpringItem>(static_cast<int>(ins.x), static_cast<int>(ins.y));     break;
                case 4:  newItem = std::make_unique<AngelWingsItem>(static_cast<int>(ins.x), static_cast<int>(ins.y)); break;
                default: newItem = std::make_unique<MushroomItem>(static_cast<int>(ins.x), static_cast<int>(ins.y));   break;
            }
            item = newItem.get();
            entities.push_back(std::move(newItem));
            items.push_back(item);
        } else {
            item = items[i];
        }
        item->rect.x     = static_cast<int>(ins.x);
        item->rect.y     = static_cast<int>(ins.y);
        item->prevRect   = item->rect;
        item->effectTimer= ins.effectTimer;
        item->hp         = static_cast<int>(ins.hp);
        if (!ins.alive)  item->kill();
        if (!ins.active) item->deactivate();
    }
 
    // sync platforms
    for (size_t i = 0; i < sup.platformActive.size() && i < platforms.size(); ++i) {
        platforms[i]->active = (sup.platformActive[i] != 0);
    }
 
    // sync countdown
    countdownTimer  = sup.countdownTimer;
    countdownActive = sup.countdownActive != 0;
 
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
    if (isDown(options.keyP1Grapple)) inputs |= InputBit::GRAPPLE;
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
    if (isDown(SDL_CONTROLLER_BUTTON_RIGHTSTICK,  0)) inputs |= InputBit::GRAPPLE;

    ClientInputPacket cip(netFrame, inputs, lastSentInputs);
    network->send(cip);
    lastSentInputs = inputs;
}