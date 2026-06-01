#include "core/Game.h"

#include "core/Resources.h"

#include <algorithm>
#include <random>


namespace {
    struct HitValues {
        int damage;
        float kbScale;
        bool critical;
    };

        HitValues rollCriticalHit(const Player& attacker, int damage, float kbScale) {
            static std::mt19937 rng(std::random_device{}());
            static std::uniform_real_distribution<float> chance(0.0f, 1.0f);

        if (chance(rng) >= attacker.character->stats.critChance) {
            return {damage, kbScale, false};
        }
        return {std::max(1, damage * 2), kbScale * 1.5f, true};
    }
}


//////////////////////////////////////
/*               GAME               */
//////////////////////////////////////

void Game::updateGameplay(float ts) {
    player1.update(platforms, isDown(options.keyP1Down) || getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 0) > 0.5f, ts);
    bool p2Down = (networkMode == NetworkMode::REMOTE_HOST)
                  ? remoteIsDown(InputBit::DOWN)
                  : (isDown(options.keyP2Down) || getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 1) > 0.5f);
    player2.update(platforms, p2Down, ts);

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
                    shockwaves.emplace_back(attacker.rect.x + attacker.rect.w / 2,
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

    auto spawnHitParticles = [&](const Player& target, const HitValues& hit) {
        float x = target.rect.x + target.rect.w / 2.0f;
        float y = target.rect.y + target.rect.h / 2.0f;
        particles.spawnDamage(x, y);
        if (hit.critical) {
            particles.spawnCrit(x, y);
        }
    };

    auto tryParryProjectile = [&](Projectile& projectile) {
        auto tryHitbox = [&](const CollisionRect& hitbox) {
            if (!hitbox.owner || projectile.owner == hitbox.owner) return false;
            if (projectile.parryFreezeTimer > 0) return false;
            if (!SDL_HasIntersection(&projectile.rect, &hitbox.rect)) return false;

            projectile.parry(hitbox.owner);
            hitbox.owner->charge = std::min(hitbox.owner->charge + 0.2f, Player::MAX_CHARGE);
            Mix_Chunk* parrySound = Resources::get().getSound("parry");
            if (parrySound) Mix_PlayChannel(-1, parrySound, 0);
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
        it->update(ts);
        if (it->rect.x >= SW || it->rect.x <= 0) { it = projectiles.erase(it); continue; }
        if (it->owner != &player1 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer > 0) { ++it; continue; }
            auto hit = rollCriticalHit(*it->owner, it->owner->character->stats.projectileDamage, 1.0f);
            if (player1.status == Status::SHIELDED && player1.shieldTimer > 0 && !player1.shieldBroken) {
                player1.blockHit(hit.damage, hit.kbScale);
                it = projectiles.erase(it);
                continue;
            } else {
                player1.getHit(it->direction, hit.damage, hit.kbScale);
                spawnHitParticles(player1, hit);
                it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
                it = projectiles.erase(it);
                continue;
            }
        }
        if (it->owner != &player2 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer > 0) { ++it; continue; }
            auto hit = rollCriticalHit(*it->owner, it->owner->character->stats.projectileDamage, 1.0f);
            if (player2.status == Status::SHIELDED && player2.shieldTimer > 0 && !player2.shieldBroken) {
                player2.blockHit(hit.damage, hit.kbScale);
                it = projectiles.erase(it);
                continue;
            } else {
                player2.getHit(it->direction, hit.damage, hit.kbScale);
                spawnHitParticles(player2, hit);
                it->owner->charge = std::min(it->owner->charge + 0.1f, Player::MAX_CHARGE);
                it = projectiles.erase(it);
                continue;
            }
        }
        ++it;
    }
    if (projectiles.size() > MAX_PROJ) projectiles.erase(projectiles.end());

    // melee hitboxes
    for (auto it = meleeHitboxes.begin(); it != meleeHitboxes.end(); ) {
        it->update(ts);
        if (!it->isAlive()) { it = meleeHitboxes.erase(it); continue; }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            auto hit = rollCriticalHit(*it->owner, it->owner->character->stats.damage, it->kbScale);
            if (player2.status == Status::SHIELDED && player2.shieldTimer > 0 && !player2.shieldBroken) {
                player2.blockHit(hit.damage, hit.kbScale);
            } else {
                player2.getHit(player1.facing, hit.damage, hit.kbScale);
                spawnHitParticles(player2, hit);
                player1.charge = std::min(player1.charge + 0.1f, Player::MAX_CHARGE);
            }
            it = meleeHitboxes.erase(it);
            continue;
        }
        if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            auto hit = rollCriticalHit(*it->owner, it->owner->character->stats.damage, it->kbScale);
            if (player1.status == Status::SHIELDED && player1.shieldTimer > 0 && !player1.shieldBroken) {
                player1.blockHit(hit.damage, hit.kbScale);
            } else {
                player1.getHit(player2.facing, hit.damage, hit.kbScale);
                spawnHitParticles(player1, hit);
                player2.charge = std::min(player2.charge + 0.1f, Player::MAX_CHARGE);
            }
            it = meleeHitboxes.erase(it);
            continue;
        }
        ++it;
    }

    // shockwaves
    for (auto it = shockwaves.begin(); it != shockwaves.end(); ) {
        it->update(ts);
        auto tryHit = [&](Player& target) {
            if (&target == it->getOwner()) return;
            if (target.invulnerableTimer > 0) return;
            auto dir = it->checkCollision(target);
            if (!dir) return;
            int dmg = static_cast<int>(it->getOwner()->character->stats.damage * 1.5f);
            float kb = 1.5f;
            auto hit = rollCriticalHit(*it->getOwner(), dmg, kb);
            if (target.status == Status::SHIELDED && target.shieldTimer > 0 && !target.shieldBroken) {
                target.blockHit(hit.damage, hit.kbScale);
            } else {
                target.getHit(*dir, hit.damage, hit.kbScale);
                spawnHitParticles(target, hit);
            }
        };
        tryHit(player1); tryHit(player2);
        if (!it->isAlive()) it = shockwaves.erase(it); else ++it;
    }

    // special hitboxes collision
    for (auto it = specialHitboxes.begin(); it != specialHitboxes.end(); ) {
        it->update(ts);
        if (!it->isAlive()) { it = specialHitboxes.erase(it); continue; }
        if (it->owner == &player1 && SDL_HasIntersection(&it->rect, &player2.rect)) {
            if (player2.invulnerableTimer == 0) {
                int dmg = static_cast<int>(player1.character->stats.damage * it->damageScale);
                auto hit = rollCriticalHit(player1, dmg, it->kbScale);
                if (player2.status == Status::SHIELDED && player2.shieldTimer > 0 && !player2.shieldBroken) {
                    player2.blockHit(hit.damage, hit.kbScale);
                } else {
                    player2.getHit(player1.facing, hit.damage, hit.kbScale);
                    spawnHitParticles(player2, hit);
                }
                it = specialHitboxes.erase(it);
                continue;
            }
        } else if (it->owner == &player2 && SDL_HasIntersection(&it->rect, &player1.rect)) {
            if (player1.invulnerableTimer == 0) {
                int dmg = static_cast<int>(player2.character->stats.damage * it->damageScale);
                auto hit = rollCriticalHit(player2, dmg, it->kbScale);
                if (player1.status == Status::SHIELDED && player1.shieldTimer > 0 && !player1.shieldBroken) {
                    player1.blockHit(hit.damage, hit.kbScale);
                } else {
                    player1.getHit(player2.facing, hit.damage, hit.kbScale);
                    spawnHitParticles(player1, hit);
                }
                it = specialHitboxes.erase(it);
                continue;
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
            Mix_Chunk* gameEndSound = Resources::get().getSound("game_end");
            if (gameEndSound) Mix_PlayChannel(-1, gameEndSound, 0);
            p.lives = -1;
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
    Mix_Chunk* deathSound = Resources::get().getSound("death");

    // play corresponding death sound
    if (voidDeath && voidDeathSound)
        Mix_PlayChannel(-1, voidDeathSound, 0);
    else if (deathSound)
        Mix_PlayChannel(-1, deathSound, 0);

    // spawn death particles before respawing
    particles.spawnDeath(p.rect.x + p.rect.w / 2, p.rect.y + p.rect.h / 2);

    // reset the player
    p.hp                 = p.character->stats.health;
    p.rect.x             = sp.x;
    p.rect.y             = sp.y - 2000;
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
