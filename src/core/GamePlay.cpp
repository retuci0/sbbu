#include "core/Game.h"

#include "core/Resources.h"

#include "entity/CollisionRect.h"
#include "entity/Grapple.h"

#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <memory>
#include <random>
#include <vector>


namespace {
    struct HitValues {
        int damage;
        float kbScale;
        bool critical;
    };

    HitValues rollCriticalHit(const Player* attacker, int damage, float kbScale) {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> chance(0.0f, 1.0f);

        if (chance(rng) >= attacker->character.stats.critChance) {
            return { damage, kbScale, false };
        }
        return { std::max(1, damage * 2), kbScale * 1.5f, true };
    }
}

//////////////////////////////////////
/*               GAME               */
//////////////////////////////////////

void Game::resetItemSpawnTimer() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(ITEM_SPAWN_MIN, ITEM_SPAWN_MAX);
    itemSpawnTimer = dist(rng);
}

void Game::trySpawnItem() {
    if (!itemsEnabled) return;

    int liveCount = 0;
    for (Item* item : items) {
        if (item->isActive()) ++liveCount;
    }
    if (liveCount >= MAX_LIVE_ITEMS) return;

    if (platforms.empty()) return;

    static std::mt19937 rng(std::random_device{}());

    struct ItemEntry {
        int weight;
        std::function<std::unique_ptr<Item>(int, int)> make;
    };
    static const std::vector<ItemEntry> itemTable = {
        { 10, [](int x, int y) { return std::make_unique<MushroomItem>(x, y); } },
        { 10, [](int x, int y) { return std::make_unique<ShitItem>(x, y);     } },
        { 6,  [](int x, int y) { return std::make_unique<CocaineItem>(x, y);  } },
        { 4,  [](int x, int y) { return std::make_unique<SpringItem>(x, y);   } },
    };

    // pick a random platform
    std::uniform_int_distribution<int> platPick(0, static_cast<int>(platforms.size()) - 1);
    Platform* plat = platforms[platPick(rng)];

    constexpr int ITEM_W = 56;
    constexpr int ITEM_H = 56;
    int margin = ITEM_W / 2;
    int spawnX = plat->rect.x + margin;
    int rangeW = plat->rect.w - ITEM_W - margin * 2;
    if (rangeW > 0) {
        std::uniform_int_distribution<int> xPick(0, rangeW);
        spawnX += xPick(rng);
    }
    int spawnY = plat->rect.y - ITEM_H;

    int totalWeight = 0;
    for (const auto& e : itemTable) totalWeight += e.weight;
    std::uniform_int_distribution<int> weightPick(0, totalWeight - 1);
    int roll = weightPick(rng);
    const ItemEntry* chosen = nullptr;
    for (const auto& e : itemTable) {
        roll -= e.weight;
        if (roll < 0) { chosen = &e; break; }
    }

    auto itemPtr = chosen->make(spawnX, spawnY);
    Item* item = itemPtr.get();
    entities.push_back(std::move(itemPtr));
    items.push_back(item);

    Mix_PlayChannel(-1, Resources::get().getSound("spawn_item"), 0);
}

void Game::updateGameplay(float ts) {
    if (timer > 0) timer -= ts;

    // item spawner
    if (!screens.current()) {
        itemSpawnTimer -= ts;
        if (itemSpawnTimer <= 0.0f) {
            trySpawnItem();
            resetItemSpawnTimer();
        }
    }

    player1->setDownKeyPressed(isDown(options.keyP1Down) || getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 0) > 0.5f);
    player1->update(entities, ts);
    bool p2Down = (networkMode == NetworkMode::REMOTE_HOST)
                  ? remoteIsDown(InputBit::DOWN)
                  : (isDown(options.keyP2Down) || getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 1) > 0.5f);
    player2->setDownKeyPressed(p2Down);
    player2->update(entities, ts);

    auto trySpawnSpecialHitbox = [&](Player* attacker) {
        if (attacker->status != Status::SPECIAL_STATIC 
                && attacker->status != Status::SPECIAL_SIDE   
                && attacker->status != Status::SPECIAL_UP     
                && attacker->status != Status::SPECIAL_DOWN
        ) {
            return;
        }
        if (attacker->specialHitboxSpawned) return;
        if (attacker->specialTimer > Player::SPECIAL_DURATION - Player::SPECIAL_HITBOX_DURATION) return;

        attacker->specialHitboxSpawned = true;

        SpecialHitboxParams p;
        switch (attacker->status) {
            case Status::SPECIAL_STATIC: p = attacker->character.specialStatic(attacker); break;
            case Status::SPECIAL_SIDE:   p = attacker->character.specialSide(attacker);   break;
            case Status::SPECIAL_UP:     p = attacker->character.specialUp(attacker);     break;
            case Status::SPECIAL_DOWN:   p = attacker->character.specialDown(attacker);   break;
            default: return;
        }

        if (p.spawnShockwave && attacker->onGround) {
            auto* shock = spawnEntity<Shockwave>(attacker->rect.x + attacker->rect.w / 2,
                                                  attacker->rect.y + attacker->rect.h - SHOCKWAVE_SIZE * attacker->scale,
                                                  attacker);
            shockwaves.push_back(shock);
        }

        auto* hitbox = spawnEntity<CollisionRect>(p.x, p.y, p.w, p.h, attacker, 5);
        hitbox->damageScale = p.damageScale;
        hitbox->kbScale     = p.kbScale;
        specialHitboxes.push_back(hitbox);
    };
    trySpawnSpecialHitbox(player1);
    trySpawnSpecialHitbox(player2);

    auto spawnHitParticles = [&](const Player& target, const HitValues& hit) {
        float x = target.rect.x + target.rect.w / 2.0f;
        float y = target.rect.y + target.rect.h / 2.0f;
        particles.spawnDamage(x, y);
        if (hit.critical) {
            particles.spawnCrit(x, y);
        }
    };

    auto tryParryProjectile = [&](Projectile* projectile) {
        auto tryHitbox = [&](CollisionRect* hitbox) {
            if (!hitbox->owner || projectile->owner == hitbox->owner) return false;
            if (projectile->parryFreezeTimer > 0) return false;
            if (!SDL_HasIntersection(&projectile->rect, &hitbox->rect)) return false;

            projectile->parry(hitbox->owner);
            hitbox->owner->charge = std::min(hitbox->owner->charge + 0.2f, Player::MAX_CHARGE);
            Mix_Chunk* parrySound = Resources::get().getSound("parry");
            if (parrySound) Mix_PlayChannel(-1, parrySound, 0);
            return true;
        };

        for (CollisionRect* hitbox : meleeHitboxes) {
            if (tryHitbox(hitbox)) return true;
        }
        for (CollisionRect* hitbox : specialHitboxes) {
            if (tryHitbox(hitbox)) return true;
        }
        return false;
    };

    // projectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        Projectile* proj = *it;
        if (tryParryProjectile(proj)) { ++it; continue; }
        proj->update(entities, ts);
        if (proj->rect.x >= SW || proj->rect.x <= 0) {
            destroyEntity(proj);
            it = projectiles.erase(it);
            continue;
        }

        // projectile vs items
        bool hitItem = false;
        for (auto itemIt = items.begin(); itemIt != items.end(); ) {
            Item* item = *itemIt;
            if (!item->isActive() || !item->isAlive()) { ++itemIt; continue; }
            if (!SDL_HasIntersection(&proj->rect, &item->rect)) { ++itemIt; continue; }
            int dmg = proj->owner->character.stats.projectileDamage;
            item->takeDamage(dmg, proj->facing, 1.0f);
            if (!item->isAlive()) {
                Player* beneficiary = proj->owner;
                item->consumer = beneficiary;
                item->rect.x = beneficiary->rect.x;
                item->rect.y = beneficiary->rect.y;
                item->onPickup();
                destroyEntity(item);
                itemIt = items.erase(itemIt);
            } else {
                ++itemIt;
            }
            destroyEntity(proj);
            it = projectiles.erase(it);
            hitItem = true;
            break;
        }
        if (hitItem) continue;

        if (proj->owner != player1 && SDL_HasIntersection(&proj->rect, &player1->rect)) {
            if (player1->invulnerableTimer > 0) { ++it; continue; }
            auto hit = rollCriticalHit(proj->owner, proj->owner->character.stats.projectileDamage, 1.0f);
            if (player1->status == Status::SHIELDED && player1->shieldTimer > 0 && !player1->shieldBroken) {
                player1->blockHit(hit.damage, hit.kbScale);
                destroyEntity(proj);
                it = projectiles.erase(it);
                continue;
            } else {
                player1->getHit(proj->owner, proj->facing, hit.damage, hit.kbScale);
                spawnHitParticles(*player1, hit);
                proj->owner->charge = std::min(proj->owner->charge + 0.1f, Player::MAX_CHARGE);
                destroyEntity(proj);
                it = projectiles.erase(it);
                continue;
            }
        }
        if (proj->owner != player2 && SDL_HasIntersection(&proj->rect, &player2->rect)) {
            if (player2->invulnerableTimer > 0) { ++it; continue; }
            auto hit = rollCriticalHit(proj->owner, proj->owner->character.stats.projectileDamage, 1.0f);
            if (player2->status == Status::SHIELDED && player2->shieldTimer > 0 && !player2->shieldBroken) {
                player2->blockHit(hit.damage, hit.kbScale);
                destroyEntity(proj);
                it = projectiles.erase(it);
                continue;
            } else {
                player2->getHit(proj->owner, proj->facing, hit.damage, hit.kbScale);
                spawnHitParticles(*player2, hit);
                proj->owner->charge = std::min(proj->owner->charge + 0.1f, Player::MAX_CHARGE);
                destroyEntity(proj);
                it = projectiles.erase(it);
                continue;
            }
        }
        ++it;
    }
    if (projectiles.size() > MAX_PROJ) {
        // remove extra projectiles from the end
        for (size_t i = MAX_PROJ; i < projectiles.size(); ++i) {
            destroyEntity(projectiles[i]);
        }
        projectiles.resize(MAX_PROJ);
    }

    // melee hitboxes
    for (auto it = meleeHitboxes.begin(); it != meleeHitboxes.end(); ) {
        CollisionRect* hitbox = *it;
        hitbox->update(entities, ts);
        if (!hitbox->isAlive()) {
            destroyEntity(hitbox);
            it = meleeHitboxes.erase(it);
            continue;
        }

        for (auto itemIt = items.begin(); itemIt != items.end(); ) {
            Item* item = *itemIt;
            if (!item->isActive() || !item->isAlive()) { ++itemIt; continue; }
            if (!SDL_HasIntersection(&hitbox->rect, &item->rect)) { ++itemIt; continue; }
            item->takeDamage(hitbox->owner->character.stats.damage, hitbox->owner->facing, hitbox->kbScale);
            if (!item->isAlive()) {
                Player* beneficiary = hitbox->owner;
                item->consumer = beneficiary;
                item->rect.x = beneficiary->rect.x;
                item->rect.y = beneficiary->rect.y;
                item->onPickup();
                destroyEntity(item);
                itemIt = items.erase(itemIt);
            } else {
                ++itemIt;
            }
            break;  // one item per hitbox
        }

        if (hitbox->owner == player1 && SDL_HasIntersection(&hitbox->rect, &player2->rect)) {
            auto hit = rollCriticalHit(hitbox->owner, hitbox->owner->character.stats.damage, hitbox->kbScale);
            if (player2->invulnerableTimer > 0) continue;
            if (player2->status == Status::SHIELDED && player2->shieldTimer > 0 && !player2->shieldBroken) {
                player2->blockHit(hit.damage, hit.kbScale);
            } else {
                player2->getHit(hitbox->owner, player1->facing, hit.damage, hit.kbScale);
                spawnHitParticles(*player2, hit);
                player1->charge = std::min(player1->charge + 0.1f, Player::MAX_CHARGE);
            }
            destroyEntity(hitbox);
            it = meleeHitboxes.erase(it);
            continue;
        }
        if (hitbox->owner == player2 && SDL_HasIntersection(&hitbox->rect, &player1->rect)) {
            if (player2->invulnerableTimer > 0) continue;
            auto hit = rollCriticalHit(hitbox->owner, hitbox->owner->character.stats.damage, hitbox->kbScale);
            if (player1->status == Status::SHIELDED && player1->shieldTimer > 0 && !player1->shieldBroken) {
                player1->blockHit(hit.damage, hit.kbScale);
            } else {
                player1->getHit(hitbox->owner, player2->facing, hit.damage, hit.kbScale);
                spawnHitParticles(*player1, hit);
                player2->charge = std::min(player2->charge + 0.1f, Player::MAX_CHARGE);
            }
            destroyEntity(hitbox);
            it = meleeHitboxes.erase(it);
            continue;
        }
        ++it;
    }

    // items
    for (auto it = items.begin(); it != items.end(); ) {
        Item* item = *it;
        item->update(entities, ts);
        if (stage.isOutsideWorld(item->rect)) item->kill();
        if (!item->isActive() && !item->isAlive() && item->effectTimer <= 0.0f) {
            destroyEntity(item);
            it = items.erase(it);
        } else {
            ++it;
        }
    }

    // shockwaves
    for (auto it = shockwaves.begin(); it != shockwaves.end(); ) {
        Shockwave* shock = *it;
        shock->update(entities, ts);
        auto tryHit = [&](Player& target) {
            if (&target == shock->getOwner()) return;
            if (target.invulnerableTimer > 0) return;
            auto dir = shock->checkCollision(target);
            if (!dir) return;
            int dmg   = static_cast<int>(shock->getOwner()->character.stats.damage * 1.5f);
            float kb  = 1.5f;
            auto hit  = rollCriticalHit(shock->getOwner(), dmg, kb);
            if (target.status == Status::SHIELDED && target.shieldTimer > 0 && !target.shieldBroken) {
                target.blockHit(hit.damage, hit.kbScale);
            } else {
                target.getHit(shock->getOwner(), *dir, hit.damage, hit.kbScale);
                spawnHitParticles(target, hit);
            }
        };
        tryHit(*player1);
        tryHit(*player2);
        if (!shock->isAlive()) {
            destroyEntity(shock);
            it = shockwaves.erase(it);
        } else {
            ++it;
        }
    }

    // special hitboxes collision
    for (auto it = specialHitboxes.begin(); it != specialHitboxes.end(); ) {
        CollisionRect* hitbox = *it;
        hitbox->update(entities, ts);
        if (!hitbox->isAlive()) {
            destroyEntity(hitbox);
            it = specialHitboxes.erase(it);
            continue;
        }

        // special vs items
        for (Item* item : items) {
            if (!item->isActive() || !item->isAlive()) continue;
            if (!SDL_HasIntersection(&hitbox->rect, &item->rect)) continue;
            int dmg = static_cast<int>(hitbox->owner->character.stats.damage * hitbox->damageScale);
            item->takeDamage(dmg, hitbox->owner->facing, hitbox->kbScale);
        }

        if (hitbox->owner == player1 && SDL_HasIntersection(&hitbox->rect, &player2->rect)) {
            if (player2->invulnerableTimer == 0) {
                int dmg  = static_cast<int>(player1->character.stats.damage * hitbox->damageScale);
                auto hit = rollCriticalHit(player1, dmg, hitbox->kbScale);
                if (player2->status == Status::SHIELDED && player2->shieldTimer > 0 && !player2->shieldBroken) {
                    player2->blockHit(hit.damage, hit.kbScale);
                } else {
                    player2->getHit(hitbox->owner, player1->facing, hit.damage, hit.kbScale);
                    spawnHitParticles(*player2, hit);
                }
                destroyEntity(hitbox);
                it = specialHitboxes.erase(it);
                continue;
            }
        } else if (hitbox->owner == player2 && SDL_HasIntersection(&hitbox->rect, &player1->rect)) {
            if (player1->invulnerableTimer == 0) {
                int dmg  = static_cast<int>(player2->character.stats.damage * hitbox->damageScale);
                auto hit = rollCriticalHit(player2, dmg, hitbox->kbScale);
                if (player1->status == Status::SHIELDED && player1->shieldTimer > 0 && !player1->shieldBroken) {
                    player1->blockHit(hit.damage, hit.kbScale);
                } else {
                    player1->getHit(hitbox->owner, player2->facing, hit.damage, hit.kbScale);
                    spawnHitParticles(*player1, hit);
                }
                destroyEntity(hitbox);
                it = specialHitboxes.erase(it);
                continue;
            }
        }
        ++it;
    }

    // death handling
    auto handleDeath = [&](Player* p) {
        bool voidDeath = stage.isOutsideWorld(p->rect);
        bool hpDead    = (p->hp <= 0);
        if (!(voidDeath || hpDead)) return;
        if (p->lives > 0) {
            respawn(*p, voidDeath);
        } else if (p->lives == 0) {
            p->lives = -1;
        }
    };
    handleDeath(player1);
    handleDeath(player2);
}

void Game::respawn(Player& p, bool voidDeath) {
    static std::mt19937 rng(std::random_device{}());
    const auto& spawns = stage.spawnpoints;
    std::uniform_int_distribution<int> pick(0, static_cast<int>(spawns.size()) - 1);
    const auto& sp = spawns[pick(rng)];

    Mix_Chunk* voidDeathSound = Resources::get().getSound("void_death");
    Mix_Chunk* deathSound     = Resources::get().getSound("death");

    if (voidDeath && voidDeathSound)
        Mix_PlayChannel(-1, voidDeathSound, 0);
    else if (deathSound)
        Mix_PlayChannel(-1, deathSound, 0);

    particles.spawnDeath(p.rect.x + p.rect.w / 2, p.rect.y + p.rect.h / 2);

    p.hp                   = p.character.stats.health;
    p.rect.x               = sp.x;
    p.rect.y               = sp.y;
    p.lives               -= 1;
    p.status               = Status::IDLE;
    p.charge               = 0.0f;
    p.dx = p.dy            = 0.0f;
    p.onGround             = false;
    p.hasAirJumped         = false;
    p.currentSpriteIndex   = 0.0f;
    p.specialHitboxSpawned = false;
    p.resetTimers();
    p.ungrapple();
    p.invulnerableTimer    = Player::INV_DURATION;
}


void Game::destroyEntity(Entity* victim) {
    if (!victim) return;

    // retract any grapple that targets this entity
    auto retractIfMatches = [victim](Player* p) {
        if (!p || !p->grapple) return;
        Grapple* g = p->grapple;
        if (g->targetPlayer == victim 
                || g->targetProjectile == victim 
                || g->targetPoint == victim 
                || g->targetItem == victim
        ) {
            g->retract();
        }
    };
    retractIfMatches(player1);
    retractIfMatches(player2);

    auto it = std::find_if(entities.begin(), entities.end(),
        [victim](const std::unique_ptr<Entity>& e) { return e.get() == victim; });
    if (it != entities.end()) entities.erase(it);
}