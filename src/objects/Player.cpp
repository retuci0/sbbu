#include "Player.h"

#include "Platform.h"
#include "../misc/Common.h"

#include "../misc/Renderer.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <string>


void Player::init(int x, int y, const Character* ch, const std::string& playerName, Mix_Chunk* dmgSound) {
    character    = ch;
    name         = playerName;
    damageSound  = dmgSound;
    hp           = ch->stats.health;
    lives        = 2;
    dx = dy      = 0.0f;
    status       = Status::IDLE;
    facing       = Facing::RIGHT;
    currentSpriteIndex = 0.0f;

    int w = 125, h = 89;
    if (ch->idle) {
        SDL_QueryTexture(ch->idle, nullptr, nullptr, &w, &h);
    }
    rect = {x, y, w, h};
}

void Player::move(int direction) {
    if (status == Status::ATTACKING || status == Status::DAMAGED) {
        dx = direction * character->stats.velocity;
        return;
    }

    if (direction > 0) {
        facing = Facing::RIGHT;
        status = Status::WALKING;
    } else if (direction < 0) {
        facing = Facing::LEFT;
        status = Status::WALKING;
    } else {
        status = Status::IDLE;
    }
    dx = direction * character->stats.velocity;
}

void Player::jump() {
    if (onGround) {
        dy = -character->stats.jumpVelocity;
        status = Status::JUMPING;
        onGround = false;
        return;
    }
    if (!hasAirJumped) {
        dy = -character->stats.jumpVelocity;
        status = Status::JUMPING;
        hasAirJumped = true;
    }
}

void Player::getHit(Facing side) {
    if (invulnerableTimer > 0) return;

    if (damageSound) { 
        Mix_PlayChannel(-1, damageSound, 0); 
    }

    status = Status::DAMAGED;
    damagedTimer = DAMAGED_DURATION;

    float damageTaken = static_cast<float>(character->stats.health - hp);
    float w = character->stats.weight;

    if (side == Facing::LEFT) {
        dx = (-damageTaken / 3.3f) * w;
    } else {
        dx = (damageTaken / 3.3f) * w;
    }
    dy = (-damageTaken / 5.0f) * w;

    charge = std::max(0.0f, charge - damageTaken * 0.05f);
}

void Player::update(const std::vector<Platform>& platforms) {
    // apply gravity
    dy = std::min(
        character->stats.terminalVelocity,
        dy + character->stats.gravity
    );

    // move horizontally
    rect.x += static_cast<int>(dx);

    // horizontal collision + edge climbing
    for (const auto& p : platforms) {
        if (SDL_HasIntersection(&rect, &p.rect)) {
            int feetY       = rect.y + rect.h;
            int platformTop = p.rect.y;
            int edgeOverlap = feetY - platformTop;

            // if feet are just barely overlapping the top of the platform,
            // step the player up onto it (edge climb) rather than blocking them
            if (edgeOverlap > 0 && edgeOverlap <= EDGE_CLIMB_THRESHOLD && dy >= 0) {
                rect.y = platformTop - rect.h;
                dy = 0;
            } else {
                if (dy >= 0) {
                    if (dx > 0) rect.x = p.rect.x - rect.w;
                    if (dx < 0) rect.x = p.rect.x + p.rect.w;
                }
            }
        }
    }

    // move vertically
    rect.y += static_cast<int>(dy);
    onGround = false;

    for (const auto& p : platforms) {
        if (SDL_HasIntersection(&rect, &p.rect)) {
            if (dy >= 0) {
                // landing on top of platform
                rect.y = p.rect.y - rect.h;
                dy = 0;
                if (!onGround) {
                    hasAirJumped = false;
                }
                onGround = true;
            }
            // dy < 0 (jumping upward): phase through — no collision
        }
    }

    if (!onGround) {
        SDL_Rect probe = { rect.x, rect.y + 2, rect.w, rect.h };
        for (const auto& p : platforms) {
            if (SDL_HasIntersection(&probe, &p.rect)) {
                onGround = true;
                break;
            }
        }
    }

    // decrement timers
    updateTimers();

    // clamp charge to [0, 1]
    charge = std::clamp(charge, 0.0f, 1.0f);

    // sync status
    if (status != Status::DAMAGED && status != Status::ATTACKING && status != Status::SHOOTING) {
        if (!onGround) {
            status = Status::JUMPING;
        } else if (dx != 0.0f) {
            status = Status::WALKING;
        } else {
            status = Status::IDLE;
        }
    }

    animate();
}

    bool Player::tryShoot(Mix_Chunk* projSound) {
        if (shootCooldown > 0) return false;
        if (projSound) Mix_PlayChannel(-1, projSound, 0);
        shootCooldown = SHOOT_COOLDOWN;
        status = Status::SHOOTING;
        return true;
    }

    bool Player::tryMelee(Mix_Chunk* meleeSound) {
        if (meleeCooldown > 0) return false;
        status = Status::ATTACKING;
        meleeTimer = MELEE_DURATION;  // hitbox active for 8 frames
        meleeCooldown = MELEE_COOLDOWN;
        currentSpriteIndex = 0.0f;
        if (meleeSound) Mix_PlayChannel(-1, meleeSound, 0);
        return true;
    }

    void Player::updateTimers() {
        if (shootCooldown > 0) --shootCooldown;
        if (meleeCooldown > 0) --meleeCooldown;
        if (invulnerableTimer > 0) --invulnerableTimer;
        if (meleeTimer > 0) {
            --meleeTimer;
            if (meleeTimer == 0 && status == Status::ATTACKING) {
                status = Status::IDLE;  // let the sync block take over next frame
            }
        }
        if (status == Status::DAMAGED) {
            if (damagedTimer > 0) {
                --damagedTimer;
            } else {
                status = Status::IDLE;  // let the sync block take over next frame
            }
        }
    }

    void Player::resetTimers() {
        shootCooldown       = 0;
        meleeCooldown       = 0;
        meleeTimer          = 0;
        damagedTimer        = 0;
        invulnerableTimer   = 0;
    }

void Player::drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH) {
    if (!tex) { return; }
    currentTexture = tex;
    SDL_Rect dst = {rect.x, rect.y, rect.w, rect.h};
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerableTimer > 0) {
        float wave = (1.0f + std::sin(invulnerableTimer * 0.15f)) * 0.5f;
        Uint8 a = static_cast<Uint8>(60 + wave * 195);
        SDL_SetTextureAlphaMod(tex, a);
    } else {
        SDL_SetTextureAlphaMod(tex, 255);
    }

    SDL_RenderCopyEx(r, tex, nullptr, &dst, 0.0, nullptr, flip);
}

void Player::animate() {
    auto advanceFrame = [&](const std::vector<SDL_Texture*>& frames, float speed) {
        currentSpriteIndex += speed;
        if (currentSpriteIndex >= static_cast<float>(frames.size())) {
            currentSpriteIndex = 0.0f;
        }
    };

    switch (status) {
        case Status::WALKING:
            advanceFrame(character->walkFrames, 0.2f);
            break;
        case Status::JUMPING:
            advanceFrame(character->jumpFrames, 0.15f);
            break;
        case Status::ATTACKING:
            advanceFrame(character->attackFrames, 0.6f);
            break;
        default:
            currentSpriteIndex = 0.0f;
            break;
    }
}

void Player::draw(SDL_Renderer* r, TTF_Font* font) {
    bool flipH = (facing == Facing::LEFT);
    int idx = static_cast<int>(currentSpriteIndex);

    switch (status) {
        case Status::WALKING:
            if (!character->walkFrames.empty()) {
                int i = idx % static_cast<int>(character->walkFrames.size());
                drawSprite(r, character->walkFrames[i], flipH);
            }
            break;
        case Status::JUMPING:
            if (!character->jumpFrames.empty()) {
                int i = idx % static_cast<int>(character->jumpFrames.size());
                drawSprite(r, character->jumpFrames[i], flipH);
            }
            break;
        case Status::ATTACKING:
            if (!character->attackFrames.empty()) {
                int i = idx % static_cast<int>(character->attackFrames.size());
                drawSprite(r, character->attackFrames[i], flipH);
            }
            break;
        case Status::SHOOTING:
            drawSprite(r, character->shoot, flipH);
            break;
        case Status::DAMAGED:
            drawSprite(r, character->damage, flipH);
            break;
        default:
            drawSprite(r, character->idle, flipH);
            break;
    }

    // nametag
    drawNametag(r, font);
}

void Player::drawNametag(SDL_Renderer* r, TTF_Font* font) const {
    int w, h;
    TTF_SizeText(font, name.c_str(), &w, &h);
    int x = rect.x + (rect.w - w) / 2;
    int y = rect.y - 40;
    Renderer::fillRect(r, x - 2, y - 2, w + 4, h + 4, { 67, 67, 67, 67 });
    Renderer::renderText(r, font, name, x, y, color);
}

void Player::drawHitbox(SDL_Renderer* r) const {
    Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, RED, 2);
}

std::string Player::getStatusName() const {
    switch (status) {
        case Status::WALKING:
            return "walking (" + std::to_string(static_cast<int>(currentSpriteIndex) + 1) + "/5)";
        case Status::JUMPING:
            return "jumping (" + std::to_string(static_cast<int>(currentSpriteIndex + 1)) + "/5)";
        case Status::ATTACKING:
            return "attacking (" + std::to_string(static_cast<int>(currentSpriteIndex + 1)) + "/5)";
        case Status::SHOOTING:
            return "shooting";
        case Status::DAMAGED:
            return "damaged";
        default:
            return "idling";
        }
}