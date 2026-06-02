#include "obj/Player.h"

#include "obj/Platform.h"
#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <string>


//////////////////////////////////
/*             INIT             */
//////////////////////////////////

void Player::init(int x, int y, const Character* ch, const std::string& playerName) {
    character           = ch;
    name                = playerName;
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
    prevRect = rect;
}


//////////////////////////////////////
/*             MOVEMENT             */
//////////////////////////////////////

void Player::move(int direction) {
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE 
        || status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN 
        || status == Status::SHOOTING || status == Status::SHIELDED
        || status == Status::STUNNED || status == Status::DAMAGED
        || status == Status::DASHING
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
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE 
        || status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN 
        || status == Status::SHOOTING || status == Status::SHIELDED
        || status == Status::STUNNED || status == Status::DASHING
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

void Player::dash() {
    if (dashCooldown > 0.0f) return;
    if (status == Status::DASHING)  return;
    if (status == Status::STUNNED || status == Status::DAMAGED 
                || status == Status::SHIELDED
    ) {
        return;
    }

    status = Status::DASHING;
    dashTimer = static_cast<float>(DASH_DURATION);
    dashCooldown = static_cast<float>(DASH_DURATION + DASH_COOLDOWN);

    float speed = character->stats.velocity * 2.8f;
    dx = (facing == Facing::RIGHT) ? speed : -speed;
    dy = 0.0f;  // cancel vertical momentum

    // small invulnerability window at the start
    invulnerableTimer = std::max(invulnerableTimer, 8.0f);

    Mix_Chunk* dashSound = Resources::get().getSound("dash");
    if (dashSound) Mix_PlayChannel(-1, dashSound, 0);
}


/////////////////////////////////////
/*             DEFENCE             */
/////////////////////////////////////

void Player::getHit(Facing side, int damage, float kbScale) {
    if (invulnerableTimer > 0) return;

    Mix_Chunk* dmgSound = Resources::get().getSound("damage");
    if (dmgSound) { 
        Mix_PlayChannel(-1, dmgSound, 0); 
    }

    if (status != Status::STUNNED) status = Status::DAMAGED;
    damagedTimer = std::max(damagedTimer, static_cast<float>(DAMAGED_DURATION));

    hp -= damage;

    float kbMult = static_cast<float>(character->stats.health - hp);
    float w = character->stats.weight;

    if (side == Facing::LEFT) {
        dx = (-kbMult / 3.3f) * w * kbScale;
    } else {
        dx = (kbMult / 3.3f) * w * kbScale;
    }
    dy = (-kbMult / 5.0f) * w * kbScale;

    // decrement charge when taking damage
    charge = std::max(0.0f, charge - damage * 0.05f);
}

void Player::setShieldHeld(bool held) {
    shieldHeld = held;
}

bool Player::tryShield() {
    if (shieldBroken || shieldStunTimer > 0 
            || !onGround || status == Status::STUNNED
    ) {
        return false;
    }

    if (shieldTimer > 0) return true;  // already active
    shieldTimer = SHIELD_DURATION;
    status = Status::SHIELDED;
    currentSpriteIndex = 0.0f;
    return true;
}

void Player::releaseShield() {
    if (shieldStunTimer > 0) return;
    shieldTimer = 0.0f;
    if (status == Status::SHIELDED)
        status = Status::IDLE;
}

void Player::breakShield() {
    Mix_Chunk* breakSound = Resources::get().getSound("shield_break");
    if (breakSound) Mix_PlayChannel(-1, breakSound, 0);
    shieldBroken = true;
    shieldBreakTimer = SHIELD_BREAK_STUN;
    status = Status::STUNNED;
    stunTimer = SHIELD_BREAK_STUN;
    shieldTimer = 0.0f;
    shieldStunTimer = 0.0f;
    releaseShield();
}

void Player::blockHit(int damage, float kbScale) {
    if (shieldBroken) return;
    if (shieldStunTimer > 0) return;

    float cost = SHIELD_HP_HIT_COST * kbScale;
    shieldHp -= cost;
    if (shieldHp <= 0.0f) {
        breakShield();
        return;
    }

    shieldStunTimer = SHIELD_STUN_DURATION;

    Mix_Chunk* blockSound = Resources::get().getSound("block");
    if (blockSound) {
        Mix_PlayChannel(-1, blockSound, 0);
    }
}

float Player::getShieldScale() const {
    if (shieldBroken) return 0.0f;
    float scale = shieldHp / SHIELD_HP_MAX;
    return std::max(scale, SHIELD_MIN_SIZE);
}


////////////////////////////////////
/*             ATTACK             */
////////////////////////////////////

bool Player::tryShoot() {
    if (shootCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE 
        || status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN 
        || status == Status::SHOOTING || status == Status::SHIELDED
        || status == Status::STUNNED
    ) {
        return false;
    }
    Mix_Chunk* projSound = Resources::get().getSound("projectile");
    if (projSound) Mix_PlayChannel(-1, projSound, 0);
    shootCooldown = SHOOT_COOLDOWN;
    shootTimer = SHOOT_DURATION;
    status = Status::SHOOTING;
    return true;
}

bool Player::tryMelee() {
    if (meleeCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE 
        || status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN 
        || status == Status::SHOOTING || status == Status::SHIELDED
        || status == Status::STUNNED
    ) {
        return false;
    }
    status = Status::ATTACKING;
    meleeTimer = MELEE_DURATION;  // hitbox active for 8 frames
    meleeCooldown = MELEE_COOLDOWN;
    currentSpriteIndex = 0.0f;
    Mix_Chunk* meleeSound = Resources::get().getSound("melee");
    if (meleeSound) Mix_PlayChannel(-1, meleeSound, 0);
    return true;
}

bool Player::trySpecial(Direction dir) {
    if (specialCooldown > 0) return false;
    if (charge < MAX_CHARGE) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE 
        || status == Status::SPECIAL_UP || status == Status::SPECIAL_DOWN
        || status == Status::SHIELDED || status == Status::STUNNED
    ) {
        return false;
    }

    const char* snd;
    switch (dir) {
        case Direction::NONE:  
            // static special - stronger punch
            dx = 0.0f;
            dy = 0.0f;
            status = Status::SPECIAL_STATIC;
            snd = "special_static";
            break;
        case Direction::LEFT:
            // fallthrough
        case Direction::RIGHT:
            // side special - dash horizontally
            dx = (facing == Facing::RIGHT ? 12.0f : -12.0f);
            dy = 0.0f;
            status = Status::SPECIAL_SIDE;
            snd = "special_side";
            break;
        case Direction::UP:
            // up special - rise quickly
            dx = 0.0f;
            dy = -14.0f;
            status = Status::SPECIAL_UP;
            snd = "special_up";
            break;
        case Direction::DOWN:
            // down special - slam downward
            dx = 0.0f;
            dy = 16.0f;
            status = Status::SPECIAL_DOWN;
            snd = "special_down";
            break;
    }

    specialTimer         = SPECIAL_DURATION;
    specialHitboxSpawned = false;
    specialCooldown      = SPECIAL_COOLDOWN;
    currentSpriteIndex   = 0.0f;

    // play sound
    Mix_Chunk* specialSound = Resources::get().getSound(snd);
    if (specialSound) Mix_PlayChannel(-1, specialSound, 0);

    charge = 0.0f;
    return true;
}


////////////////////////////////////
/*             UPDATE             */
////////////////////////////////////

void Player::update(const std::vector<Platform>& platforms, bool downKeyPressed, float ts) {
    prevRect = rect;

    if (status == Status::SHIELDED) { dx = 0.0f; }

    // gravity
    dy = std::min(character->stats.terminalVelocity, dy + character->stats.gravity * ts);
    // reduce gravity when dashing
    if (status == Status::DASHING) dy *= 0.3f;
    
    
    // horizontal move
    rect.x += static_cast<int>(dx * ts);
    
    // snapshot bottom before vertical move
    const int prevBottom = rect.y + rect.h;
    
    // vertical move
    rect.y += static_cast<int>(dy * ts);
    onGround = false;
    
    // drop-through
    bool standingOnBig = false;
    bool dropping = (droppingTimer > 0);
    if (!dropping && dy >= 0) {
        for (const auto& p : platforms) {
            if (prevBottom > p.rect.y) continue;
            if (!SDL_HasIntersection(&rect, &p.rect)) continue;
            rect.y = p.rect.y - rect.h;
            dy = 0.0f;
            onGround = true;
            hasAirJumped = false;
            if (p.size == PlatformSize::BIG)
                standingOnBig = true;
        }
    }
    // stationary probe (only when not dropping)
    if (!onGround && !dropping) {
        SDL_Rect probe = { rect.x, rect.y + rect.h, rect.w, 3 };
        for (const auto& p : platforms) {
            if (SDL_HasIntersection(&probe, &p.rect)) {
                onGround = true;
                if (p.size == PlatformSize::BIG)
                    standingOnBig = true;
                break;
            }
        }
    }
    if (downKeyPressed && onGround && droppingTimer == 0 && !standingOnBig) {
        droppingTimer = DROP_DURATION;
    }
    if (droppingTimer > 0) droppingTimer -= ts;

    
    updateTimers(ts);

    if (currentTexture) {
        int w, h;
        SDL_QueryTexture(currentTexture, nullptr, nullptr, &w, &h);
        if (facing == Facing::LEFT && w != rect.w) {
            rect.x -= w - rect.w;
        }
        rect.w = w;
        rect.h = h;
    }

    charge = std::clamp(charge, 0.0f, 1.0f);

    if (status != Status::DAMAGED && status != Status::ATTACKING
            && status != Status::SHIELDED && status != Status::SHOOTING
            && status != Status::SPECIAL_STATIC && status != Status::SPECIAL_SIDE
            && status != Status::SPECIAL_UP && status != Status::SPECIAL_DOWN
            && status != Status::STUNNED && status != Status::DASHING
    ) {
        if (!onGround) status = Status::JUMPING;
        else if (dx != 0.0f) status = Status::WALKING;
        else status = Status::IDLE;
    }

    animate(ts);
}

void Player::updateTimers(float ts) {
    if (shootCooldown > 0.0f) shootCooldown -= ts;
    if (meleeCooldown > 0.0f) meleeCooldown -= ts;
    if (specialCooldown > 0.0f) specialCooldown -= ts;
    if (invulnerableTimer > 0.0f) invulnerableTimer -= ts;
    if (shootTimer > 0.0f) {
        shootTimer -= ts;
        if (shootTimer <= 0.0f && status == Status::SHOOTING) {
            status = Status::IDLE;  // let the sync block take over next frame
        }
    }
    if (meleeTimer > 0.0f) {
        meleeTimer -= ts;
        if (meleeTimer <= 0.0f && status == Status::ATTACKING) {
            status = Status::IDLE;
        }
    }
    if (status == Status::DAMAGED) {
        if (damagedTimer > 0.0f) {
            damagedTimer -= ts;
        } else {
            status = Status::IDLE;
        }
    }
    if (specialTimer > 0.0f) {
        specialTimer -= ts;
        if (specialTimer <= 0.0f && (status == Status::SPECIAL_STATIC 
            || status == Status::SPECIAL_SIDE
            || status == Status::SPECIAL_UP
            || status == Status::SPECIAL_DOWN)
        ) {
            status = Status::IDLE;
        }
    }
    if (stunTimer > 0.0f) {
        stunTimer -= ts;
        if (stunTimer <= 0.0f && status == Status::STUNNED) {
            status = Status::IDLE;
            shieldBroken = false;
            shieldHp = SHIELD_HP_MAX;
        }
    }
    if (shieldTimer > 0.0f) {
        shieldTimer -= ts;
        if (shieldTimer <= 0.0f && status == Status::SHIELDED)
            releaseShield();
        shieldHp -= SHIELD_HP_DRAIN * ts;
        if (shieldHp <= 0.0f && !shieldBroken) {
            breakShield();
            return;
        }
    } else {
        if (!shieldBroken && shieldHp < SHIELD_HP_MAX)
            shieldHp += SHIELD_HP_REGEN * ts;
        if (shieldHp > SHIELD_HP_MAX) shieldHp = SHIELD_HP_MAX;
    }

    if (shieldStunTimer > 0.0f) {
        shieldStunTimer -= ts;
        if (shieldStunTimer <= 0.0f && status == Status::SHIELDED) {
            if (!shieldHeld) releaseShield();
        }
    }

    if (shieldBreakTimer > 0.0f) {
        shieldBreakTimer -= ts;
        if (shieldBreakTimer <= 0.0f) {
            shieldBroken = false;
            shieldHp = SHIELD_HP_MAX;
            if (status == Status::DAMAGED && damagedTimer <= 0.0f) {
                status = Status::IDLE;  // ...
            }
        }
    }

    // autorelease shield
    if (shieldTimer > 0.0f && !shieldHeld && shieldStunTimer <= 0.0f) {
        releaseShield();
    }

    if (dashTimer > 0.0f) {
        dashTimer -= ts;
        if (dashTimer <= 0.0f) {
            dashTimer = 0.0f;
            if (status == Status::DASHING) {
                status = onGround ? Status::IDLE : Status::JUMPING;
                dx *= 0.25f;
            }
        }
    }
    if (dashCooldown > 0.0f) {
        dashCooldown = std::max(0.0f, dashCooldown - ts);
    }
}

void Player::resetTimers() {
    shootCooldown        = 0.0f;
    meleeCooldown        = 0.0f;
    specialCooldown      = 0.0f;
    meleeTimer           = 0.0f;
    damagedTimer         = 0.0f;
    invulnerableTimer    = 0.0f;
    specialTimer         = 0.0f;
    shieldTimer          = 0.0f;
    shieldBreakTimer     = 0.0f;
    shieldStunTimer      = 0.0f;
    shieldHp             = SHIELD_HP_MAX;
    shieldBroken         = false;
    shieldHeld           = false;
    specialHitboxSpawned = false;
    dashTimer            = 0.0f;
    dashCooldown         = 0.0f;
}


///////////////////////////////////////
/*             RENDERING             */
///////////////////////////////////////

void Player::drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH, float a) {
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    currentTexture = tex;
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerableTimer > 0) {
        float wave = (1.0f + std::sin(invulnerableTimer * 0.08f)) * 0.5f;
        Uint8 alpha = static_cast<Uint8>(120 + wave * 135);
        SDL_SetTextureAlphaMod(tex, alpha);
    } else {
        SDL_SetTextureAlphaMod(tex, 255);
    }

    Renderer::drawSprite(r, tex, &drawRect, flipH);
    SDL_SetTextureAlphaMod(tex, 255);
}

void Player::animate(float ts) {
    auto advanceFrame = [&](const std::vector<SDL_Texture*>& frames, float speed) {
        currentSpriteIndex += speed * ts;
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
        case Status::STUNNED:
            advanceFrame(character->specialDownFrames, 0.21f);
            break;
        case Status::DASHING:  // walk but faster
            advanceFrame(character->walkFrames, 0.45f);
            break;
        default:
            currentSpriteIndex = 0.0f;
            break;
    }
}

void Player::draw(SDL_Renderer* r, TTF_Font* font, float a) {
    bool flipH = (facing == Facing::LEFT);
    auto drawAnimatedSprite = [&](const std::vector<SDL_Texture*>& frames) -> void {
        if (frames.empty()) return;
        int i = static_cast<int>(currentSpriteIndex) % static_cast<int>(frames.size());
        drawSprite(r, frames[i], flipH, a);
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
        case Status::STUNNED:
            drawAnimatedSprite(character->stunnedFrames);
            break;
        case Status::DASHING:
            drawAnimatedSprite(character->walkFrames);
            break;
        case Status::SHOOTING:
            drawSprite(r, character->shoot, flipH, a);
            break;
        case Status::DAMAGED:
            drawSprite(r, character->damage, flipH, a);
            break;
        case Status::SHIELDED:
            drawSprite(r, character->shielded, flipH, a);
            break;
        default:
            drawSprite(r, character->idle, flipH, a);
            break;
    }

    // nametag
    drawShield(r, a);
    drawNametag(r, font, a);
}

void Player::drawShield(SDL_Renderer* r, float a) const {
    if (status != Status::SHIELDED && shieldTimer <= 0.0f) return;
    if (shieldBroken) return;
    float scale = getShieldScale();
    if (scale <= 0.0f) return;

    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    int cx = drawRect.x + drawRect.w / 2;
    int cy = drawRect.y + drawRect.h / 2;
    int radius = static_cast<int>((drawRect.w / 2.0f + drawRect.h / 2.0f) / 2 * scale);
    Color shieldColor = color;
    shieldColor.a = 128;
    Renderer::fillCircle(r, cx, cy, radius, shieldColor);
}

void Player::drawNametag(SDL_Renderer* r, TTF_Font* font, float a) const {
    int w, h;
    TTF_SizeText(font, name.c_str(), &w, &h);
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    int x = drawRect.x + (rect.w - w) / 2;
    int y = drawRect.y - 40;
    Renderer::fillRect(r, x - 2, y - 2, w + 4, h + 4, { 67, 67, 67, 67 });
    Renderer::renderText(r, font, name, x, y, color);
}

void Player::drawHitbox(SDL_Renderer* r, float a) const {
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    Renderer::outlineRect(r, drawRect.x, drawRect.y, drawRect.w, drawRect.h, RED, 2);
}


////////////////////////////////////////////
/*             HELPER METHODS             */
////////////////////////////////////////////

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
            return "special (static) (" + std::to_string(static_cast<int>(currentSpriteIndex) + 1) + "/6)";
        case Status::SPECIAL_DOWN:
            return "special (down) (" + std::to_string(static_cast<int>(currentSpriteIndex) + 1) + "/3)";
        case Status::SHIELDED:
            return "shielded";
        case Status::STUNNED:
            return "stunned";
        default:
            return "idling";
        }
}
