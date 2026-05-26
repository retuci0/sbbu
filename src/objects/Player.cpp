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
    character           = ch;
    name                = playerName;
    damageSound         = dmgSound;
    hp                  = ch->stats.health;
    lives               = 2;
    dx = dy             = 0.0f;
    status              = Status::IDLE;
    facing              = Facing::RIGHT;
    currentTexture      = character->idle;
    currentSpriteIndex  = 0.0f;

    int w = 125, h = 89;
    if (ch->idle) {
        SDL_QueryTexture(ch->idle, nullptr, nullptr, &w, &h);
    }
    rect = {x, y, w, h};
}

void Player::move(int direction) {
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE ||
        status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN ||
        status == Status::SHOOTING
    ) {
        return;
    }

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
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE ||
        status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN ||
        status == Status::SHOOTING
    ) {
        return;
    }
    
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

void Player::getHit(Facing side, int damage, float kbScale) {
    if (invulnerableTimer > 0) return;

    if (damageSound) { 
        Mix_PlayChannel(-1, damageSound, 0); 
    }

    status = Status::DAMAGED;
    damagedTimer = DAMAGED_DURATION;

    hp -= damage;

    float kbMult = static_cast<float>(character->stats.health - hp);
    float w = character->stats.weight;

    if (side == Facing::LEFT) {
        dx = (-kbMult / 3.3f) * w * kbScale;
    } else {
        dx = (kbMult / 3.3f) * w * kbScale;
    }
    dy = (-kbMult / 5.0f) * w * kbScale;

    charge = std::max(0.0f, charge - damage * 0.05f);
}

bool Player::tryShoot(Mix_Chunk* projSound) {
    if (shootCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE ||
        status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN ||
        status == Status::SHOOTING
    ) {
        return false;
    }
    if (projSound) Mix_PlayChannel(-1, projSound, 0);
    shootCooldown = SHOOT_COOLDOWN;
    shootTimer = SHOOT_DURATION;
    status = Status::SHOOTING;
    return true;
}

bool Player::tryMelee(Mix_Chunk* meleeSound) {
    if (meleeCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE ||
        status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN ||
        status == Status::SHOOTING
    ) {
        return false;
    }
    status = Status::ATTACKING;
    meleeTimer = MELEE_DURATION;  // hitbox active for 8 frames
    meleeCooldown = MELEE_COOLDOWN;
    currentSpriteIndex = 0.0f;
    if (meleeSound) Mix_PlayChannel(-1, meleeSound, 0);
    return true;
}

bool Player::trySpecial(Mix_Chunk** sounds, Direction dir) {
    if (specialCooldown > 0) return false;
    if (charge < MAX_CHARGE) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE ||
        status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN) {
        return false;
    }

    switch (dir) {
        case Direction::NONE:  
            // static special - stronger punch
            dx = 0.0f;
            dy = 0.0f;
            status = Status::SPECIAL_STATIC;
            break;
        case Direction::LEFT:
            // fallthrough
        case Direction::RIGHT:
            // side special - dash horizontally
            dx = (facing == Facing::RIGHT ? 12.0f : -12.0f);
            dy = 0.0f;
            status = Status::SPECIAL_SIDE;
            break;
        case Direction::UP:
            // up special - rise quickly
            dx = 0.0f;
            dy = -14.0f;
            status = Status::SPECIAL_UP;
            break;
        case Direction::DOWN:
            // down special - slam downward
            dx = 0.0f;
            dy = 16.0f;
            status = Status::SPECIAL_DOWN;
            break;
    }

    specialTimer         = SPECIAL_DURATION;
    specialHitboxSpawned = false;
    specialCooldown      = SPECIAL_COOLDOWN;
    currentSpriteIndex   = 0.0f;

    // play sound
    if (sounds) {
        int soundIndex = (dir == Direction::NONE) ? 0 :
                         (dir == Direction::LEFT || dir == Direction::RIGHT) ? 1 :
                         (dir == Direction::UP) ? 2 : 3;
        if (sounds[soundIndex]) Mix_PlayChannel(-1, sounds[soundIndex], 0);
    }

    charge = 0.0f;
    return true;
}

void Player::update(const std::vector<Platform>& platforms, bool downKeyPressed) {
    // apply gravity
    dy = std::min(character->stats.terminalVelocity, dy + character->stats.gravity);

    // drop-through
    if (downKeyPressed && onGround && droppingTimer == 0) droppingTimer = DROP_DURATION;
    if (droppingTimer > 0) --droppingTimer;
    const bool dropping = (droppingTimer > 0);

    // horizontal move
    rect.x += static_cast<int>(dx);

    // snapshot the player's bottom before the vertical move
    // to know whether it approached from above
    const int prevBottom = rect.y + rect.h;

    // vertical move
    rect.y += static_cast<int>(dy);
    onGround = false;

    // land on top of platforms (only while falling, only from above, only if not dropping)
    if (!dropping && dy >= 0) {
        for (const auto& p : platforms) {
            // prevents sticking from underneath when jumping up
            if (prevBottom > p.rect.y) continue;

            if (!SDL_HasIntersection(&rect, &p.rect)) continue;

            rect.y         = p.rect.y - rect.h;
            dy             = 0.0f;
            onGround       = true;
            hasAirJumped   = false;
        }
    }

    // when stationary on a platform
    if (!onGround && !dropping) {
        SDL_Rect probe = { rect.x, rect.y + rect.h, rect.w, 3 };
        for (const auto& p : platforms) {
            if (SDL_HasIntersection(&probe, &p.rect)) {
                onGround = true;
                break;
            }
        }
    }

    // decrement timers
    updateTimers();

    // adapt hitbox to current sprite's size
    int w, h;
    SDL_QueryTexture(currentTexture, nullptr, nullptr, &w, &h);
    if (facing == Facing::LEFT && w != rect.w) {
        rect.x -= w - rect.w;
    }
    rect.w = w;
    rect.h = h;

    // clamp charge to [0, 1]
    charge = std::clamp(charge, 0.0f, 1.0f);

    // sync status
    if (status != Status::DAMAGED && status != Status::ATTACKING &&
        status != Status::SHOOTING &&
        status != Status::SPECIAL_STATIC && status != Status::SPECIAL_SIDE &&
        status != Status::SPECIAL_UP && status != Status::SPECIAL_DOWN) {
        if (!onGround) status = Status::JUMPING;
        else if (dx != 0.0f) status = Status::WALKING;
        else status = Status::IDLE;
    }

    animate();
}

void Player::updateTimers() {
    if (shootCooldown > 0) --shootCooldown;
    if (meleeCooldown > 0) --meleeCooldown;
    if (specialCooldown > 0) --specialCooldown;
    if (invulnerableTimer > 0) --invulnerableTimer;
    if (shootTimer > 0) {
        --shootTimer;
        if (shootTimer == 0 && status == Status::SHOOTING) {
            status = Status::IDLE;  // let the sync block take over next frame
        }
    }
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
    if (specialTimer > 0) {
        --specialTimer;
        if (specialTimer == 0 && (status == Status::SPECIAL_STATIC 
            || status == Status::SPECIAL_SIDE
            || status == Status::SPECIAL_UP
            || status == Status::SPECIAL_DOWN)
        ) {
            status = Status::IDLE;  // let the sync block take over next frame
        }
    }
}

void Player::resetTimers() {
    shootCooldown       = 0;
    meleeCooldown       = 0;
    specialCooldown     = 0;
    meleeTimer          = 0;
    damagedTimer        = 0;
    invulnerableTimer   = 0;
    specialTimer        = 0;
    specialHitboxSpawned = false;
}

void Player::drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH) {
    if (!tex) { return; }
    currentTexture = tex;
    SDL_Rect dst = {rect.x, rect.y, rect.w, rect.h};
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerableTimer > 0) {
        float wave = (1.0f + std::sin(invulnerableTimer * 0.08f)) * 0.5f;
        Uint8 a = static_cast<Uint8>(120 + wave * 135);
        SDL_SetTextureAlphaMod(tex, a);
    } else {
        SDL_SetTextureAlphaMod(tex, 255);
    }

    SDL_RenderCopyEx(r, tex, nullptr, &dst, 0.0, nullptr, flip);
    SDL_SetTextureAlphaMod(tex, 255);
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
        case Status::SPECIAL_STATIC:
            advanceFrame(character->specialStaticFrames, 0.3f);
            break;
        case Status::SPECIAL_SIDE:
            advanceFrame(character->specialSideFrames, 0.5f);
            break;
        case Status::SPECIAL_UP:
            advanceFrame(character->specialUpFrames, 0.5f);
            break;
        case Status::SPECIAL_DOWN:
            advanceFrame(character->specialDownFrames, 0.21f);
            break;
        default:
            currentSpriteIndex = 0.0f;
            break;
    }
}

void Player::draw(SDL_Renderer* r, TTF_Font* font) {
    bool flipH = (facing == Facing::LEFT);
    auto drawAnimatedSprite = [&](const std::vector<SDL_Texture*>& frames) -> void {
        if (frames.empty()) return;
        int i = static_cast<int>(currentSpriteIndex) % static_cast<int>(frames.size());
        drawSprite(r, frames[i], flipH);
    };

    switch (status) {
        case Status::WALKING:
            drawAnimatedSprite(character->walkFrames);
            break;
        case Status::JUMPING:
            drawAnimatedSprite(character->jumpFrames);
            break;
        case Status::ATTACKING:
            drawAnimatedSprite(character->attackFrames);
            break;
        case Status::SPECIAL_STATIC:
            drawAnimatedSprite(character->specialStaticFrames);
            break;
        case Status::SPECIAL_SIDE:
            drawAnimatedSprite(character->specialSideFrames);
            break;
        case Status::SPECIAL_UP:
            drawAnimatedSprite(character->specialUpFrames);
            break;
        case Status::SPECIAL_DOWN:
            drawAnimatedSprite(character->specialDownFrames);
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
        case Status::SPECIAL_STATIC:
            return "special (static) " + std::to_string(static_cast<int>(currentSpriteIndex) + 1) + "/6)";
        default:
            return "idling";
        }
}
