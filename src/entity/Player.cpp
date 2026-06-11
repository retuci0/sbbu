#include "entity/Player.h"

#include "entity/Items.h"
#include "entity/Platform.h"
#include "entity/Grapple.h"

#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_log.h>
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

void Player::init(int x, int y, const Character ch, const std::string& playerName) {
    character           = ch;
    name                = playerName;
    hp                  = ch.stats.health;
    lives               = 2;
    dx = dy             = 0.0f;
    status              = Status::IDLE;
    facing              = Facing::RIGHT;

    tex                 = character.idle;
    currentSpriteIndex  = 0.0f;

    shieldTex = Resources::get().getTexture("shield");

    int w = 125, h = 89;
    if (ch.idle) {
        SDL_QueryTexture(ch.idle, nullptr, nullptr, &w, &h);
    }
    rect = { x, y, w, h };
    prevRect = rect;
}


//////////////////////////////////////
/*             MOVEMENT             */
//////////////////////////////////////

void Player::move(int direction) {
    // don't fight the grapple
    if (grapple && grapple->isLatched()) return;
    if (postGrappleTimer > 0.0f) return;

    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE
        || status == Status::SPECIAL_UP   || status == Status::SPECIAL_DOWN
        || status == Status::SHOOTING     || status == Status::SHIELDED
        || status == Status::STUNNED      || status == Status::DAMAGED
        || status == Status::DASHING
    ) {
        return;
    }

    if (direction == 0) return;

    if (direction > 0) facing = Facing::RIGHT;
    else facing = Facing::LEFT;

    float maxSpeed = character.stats.velocity;
    dx += direction * character.stats.acceleration;
    dx  = std::clamp(dx, -maxSpeed, maxSpeed);
}

void Player::jump() {
    // cancel grapple
    if (grapple && grapple->isLatched()) grapple->retract();

    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE
            || status == Status::SPECIAL_UP  || status == Status::SPECIAL_DOWN
            || status == Status::SHOOTING    || status == Status::SHIELDED
            || status == Status::STUNNED     || status == Status::DASHING
    ) {
        return;
    }

    if (onGround) {
        dy = -character.stats.jumpVelocity;
        status = Status::JUMPING;
        onGround = false;
        return;
    }
    if (!hasAirJumped) {
        dy = -character.stats.jumpVelocity;
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
    dashTimer    = static_cast<float>(DASH_DURATION);
    dashCooldown = static_cast<float>(DASH_DURATION + DASH_COOLDOWN);

    float speed = character.stats.velocity * 2.8f;
    dx  = (facing == Facing::RIGHT) ? speed : -speed;  // hard override — dash is an intentional burst
    dy  = 0.0f;  // cancel vertical momentum

    // small invulnerability window at the start
    invulnerableTimer = std::max(invulnerableTimer, 8.0f);

    Mix_Chunk* dashSound = Resources::get().getSound("dash");
    if (dashSound) Mix_PlayChannel(-1, dashSound, 0);
}

void Player::throwGrapple() {
    if (status == Status::STUNNED) return;
    if (grapple) { ungrapple(); }

    constexpr int HOOK_W = 14;
    constexpr int HOOK_H = 14;

    int spawnX = (facing == Facing::RIGHT)
                    ? rect.x + rect.w
                    : rect.x - HOOK_W;

    int spawnY = rect.y + (rect.h - HOOK_H) / 2;

    float velX = (facing == Facing::RIGHT)
                    ?  Grapple::TRAVEL_SPEED
                    : -Grapple::TRAVEL_SPEED;
    float velY = 0.0f;

    grapple = new Grapple(*this, spawnX, spawnY, velX, velY);
}

void Player::ungrapple() {
    delete grapple;
    grapple = nullptr;
}

/////////////////////////////////////
/*             DEFENCE             */
/////////////////////////////////////

void Player::getHit(Player* attacker, Facing side, int damage, float kbScale) {
    if (invulnerableTimer > 0) return;

    Mix_Chunk* dmgSound = Resources::get().getSound("damage");
    if (dmgSound) {
        Mix_PlayChannel(-1, dmgSound, 0);
    }

    if (status != Status::STUNNED) status = Status::DAMAGED;
    damagedTimer = std::max(damagedTimer, static_cast<float>(DAMAGED_DURATION));

    hp -= damage;

    float kbMult = static_cast<float>(character.stats.health - hp);
    float w = character.stats.weight;

    if (side == Facing::LEFT) {
        dx += (-kbMult / 3.3f) * w * kbScale;
    } else {
        dx += (kbMult / 3.3f) * w * kbScale;
    }
    dy = (-kbMult / 5.0f) * w * kbScale;

    // decrement charge when taking damage
    charge = std::max(0.0f, charge - damage * 0.05f);

    // thorns
    if (shitAuraTimer > 0.0f && attacker) {
        attacker->getHit(this, facing, damage, kbScale);
    }
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
    shieldBroken     = true;
    shieldBreakTimer = SHIELD_BREAK_STUN;
    status           = Status::STUNNED;
    stunTimer        = SHIELD_BREAK_STUN;
    dx               = 0.0f;
    dy               = 0.0f;
    shieldTimer      = 0.0f;
    shieldStunTimer  = 0.0f;
    releaseShield();
}

void Player::blockHit(int damage, float kbScale) {
    if (shieldBroken)      return;
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
    return shieldHp / SHIELD_HP_MAX;
}


bool Player::shouldSuffocate(const std::vector<const Platform*>& platforms) const {
    for (auto& platform : platforms) {
        if (rect.x > platform->rect.x && rect.x + rect.w < platform->rect.x + platform->rect.w
                && rect.y > platform->rect.y && rect.y + rect.h < platform->rect.y + platform->rect.h
        ) {
            return true;
        }
    }
    return false;
}

////////////////////////////////////
/*             ATTACK             */
////////////////////////////////////

bool Player::tryShoot() {
    if (shootCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE
        || status == Status::SPECIAL_UP   || status == Status::SPECIAL_DOWN
        || status == Status::SHOOTING     || status == Status::SHIELDED
        || status == Status::STUNNED
    ) {
        return false;
    }
    Mix_Chunk* projSound = Resources::get().getSound("projectile");
    if (projSound) Mix_PlayChannel(-1, projSound, 0);
    shootCooldown = SHOOT_COOLDOWN;
    shootTimer    = SHOOT_DURATION;
    status        = Status::SHOOTING;
    return true;
}

bool Player::tryMelee() {
    if (meleeCooldown > 0) return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE
        || status == Status::SPECIAL_UP   || status == Status::SPECIAL_DOWN
        || status == Status::SHOOTING     || status == Status::SHIELDED
        || status == Status::STUNNED
    ) {
        return false;
    }
    status             = Status::ATTACKING;
    meleeTimer         = MELEE_DURATION;
    meleeCooldown      = MELEE_COOLDOWN;
    currentSpriteIndex = 0.0f;
    Mix_Chunk* meleeSound = Resources::get().getSound("melee");
    if (meleeSound) Mix_PlayChannel(-1, meleeSound, 0);
    return true;
}

bool Player::trySpecial(Direction dir) {
    if (specialCooldown > 0)   return false;
    if (charge < MAX_CHARGE)   return false;
    if (status == Status::SPECIAL_STATIC || status == Status::SPECIAL_SIDE
        || status == Status::SPECIAL_UP   || status == Status::SPECIAL_DOWN
        || status == Status::SHIELDED     || status == Status::STUNNED
    ) {
        return false;
    }

    const char* snd;
    switch (dir) {
        case Direction::NONE:
            character.onSpecialStatic(this);
            status = Status::SPECIAL_STATIC;
            snd    = "special_static";
            break;
        case Direction::LEFT:
            // fallthrough
        case Direction::RIGHT:
            character.onSpecialSide(this);
            status = Status::SPECIAL_SIDE;
            snd    = "special_side";
            break;
        case Direction::UP:
            character.onSpecialUp(this);
            status = Status::SPECIAL_UP;
            snd    = "special_up";
            break;
        case Direction::DOWN:
            character.onSpecialDown(this);
            status = Status::SPECIAL_DOWN;
            snd    = "special_down";
            break;
    }

    specialTimer         = SPECIAL_DURATION;
    specialHitboxSpawned = false;
    specialCooldown      = SPECIAL_COOLDOWN;
    currentSpriteIndex   = 0.0f;

    Mix_Chunk* specialSound = Resources::get().getSound(snd);
    if (specialSound) Mix_PlayChannel(-1, specialSound, 0);

    charge = 0.0f;
    return true;
}


////////////////////////////////////
/*             UPDATE             */
////////////////////////////////////

void Player::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    prevRect = rect;

    // don't allow movement while shielding
    if (status == Status::SHIELDED) { dx = 0.0f; }

    // freefall
    if (downKeyPressed && (status == Status::WALKING 
            || status == Status::JUMPING 
            || status == Status::IDLE)
    ) {
        dy += 2.3;
    }

    // gravity
    if (!(grapple && grapple->isLatched())) {
        dy = std::min(character.stats.terminalVelocity, dy + character.stats.gravity * ts);
    }
    // reduce gravity when dashing
    if (status == Status::DASHING) dy *= 0.3f;

    // friction / air drag
    if (status != Status::DASHING && status != Status::DAMAGED) {
        float drag = onGround ? character.stats.friction : character.stats.airDrag;
        dx *= drag;
        if (std::abs(dx) < 0.05f) dx = 0.0f;  // snap to rest
    }

    // horizontal move
    rect.x += static_cast<int>(dx * ts);

    // snapshot bottom before vertical move
    const int prevBottom = rect.y + rect.h;

    // vertical move
    rect.y += static_cast<int>(dy * ts);
    onGround = false;

    // update grapple
    if (grapple) {
        grapple->update(entities, ts);
        if (!grapple->isAlive()) {
            if (grapple->targetPoint && grapple->targetPoint->type == GrapplePointType::BLUE)
                postGrappleTimer = POST_GRAPPLE_DURATION;
            ungrapple();
        }
    }

    std::vector<const Platform*> platforms;
    for (auto& e : entities) {
        if (auto* p = dynamic_cast<Platform*>(e.get())) {
            platforms.push_back(p);
        }
    }

    if (shouldSuffocate(platforms)) {
        getHit(nullptr, facing, SUFFOCATION_DAMAGE, 0);
    }

    // drop-through
    bool standingOnBig = false;
    bool dropping = (droppingTimer > 0);
    if (!dropping && dy >= 0) {
        for (const auto& p : platforms) {
            if (prevBottom > p->rect.y) continue;
            if (!p->intersectsWith(*this)) continue;
            rect.y = p->rect.y - rect.h;
            dy = 0.0f;
            onGround = true;
            hasAirJumped = false;
            if (p->size == PlatformSize::BIG)
                standingOnBig = true;
        }
    }
    // stationary probe (only when not dropping)
    if (!onGround && !dropping) {
        SDL_Rect probe = { rect.x, rect.y + rect.h, rect.w, 3 };
        for (const auto& p : platforms) {
            if (SDL_HasIntersection(&probe, &p->rect)) {
                onGround = true;
                if (p->size == PlatformSize::BIG) {
                    standingOnBig = true;
                }
                break;
            }
        }
    }
    if (downKeyPressed && onGround && droppingTimer == 0 && !standingOnBig) {
        droppingTimer = DROP_DURATION;
    }
    if (droppingTimer > 0) droppingTimer -= ts;

    updateTimers(ts);

    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        w *= scale;
        h *= scale;
        if (facing == Facing::LEFT && w != rect.w) {
            rect.x -= w - rect.w;
        }
        if (h != rect.h) {
            rect.y -= h - rect.h;
        }
        rect.w = w;
        rect.h = h;
    }

    charge = std::clamp(charge, 0.0f, 1.0f);

    if (status != Status::DAMAGED   && status != Status::ATTACKING
        && status != Status::SHIELDED  && status != Status::SHOOTING
        && status != Status::SPECIAL_STATIC && status != Status::SPECIAL_SIDE
        && status != Status::SPECIAL_UP     && status != Status::SPECIAL_DOWN
        && status != Status::STUNNED        && status != Status::DASHING
    ) {
        if (!onGround)              status = Status::JUMPING;
        else if (std::abs(dx) > 0.3f) status = Status::WALKING;
        else                        status = Status::IDLE;
    }

    animate(ts);
}

void Player::updateTimers(float ts) {
    if (shootCooldown > 0.0f)    shootCooldown -= ts;
    if (meleeCooldown > 0.0f)    meleeCooldown -= ts;
    if (specialCooldown > 0.0f)  specialCooldown -= ts;
    if (invulnerableTimer > 0.0f) invulnerableTimer -= ts;
    if (shootTimer > 0.0f) {
        shootTimer -= ts;
        if (shootTimer <= 0.0f && status == Status::SHOOTING) {
            status = Status::IDLE;
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
            shieldHp     = SHIELD_HP_MAX;
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
            shieldHp     = SHIELD_HP_MAX;
            if (status == Status::DAMAGED && damagedTimer <= 0.0f) {
                status = Status::IDLE;
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

    if (postGrappleTimer > 0.0f) {
        postGrappleTimer = std::max(0.0f, postGrappleTimer - ts);
    }

    if (shitAuraTimer > 0.0f) {
        shitAuraTimer -= ts;
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
    shitAuraTimer        = 0.0f;
}


///////////////////////////////////////
/*             RENDERING             */
///////////////////////////////////////

void Player::drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH, float a) {
    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    this->tex = tex;
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (invulnerableTimer > 0) {
        float wave  = (1.0f + std::sin(invulnerableTimer * 0.08f)) * 0.5f;
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
            advanceFrame(character.walkFrames, 0.2f);
            break;
        case Status::JUMPING:
            advanceFrame(character.jumpFrames, 0.15f);
            break;
        case Status::ATTACKING:
            advanceFrame(character.attackFrames, 0.6f);
            break;
        case Status::SPECIAL_STATIC:
            advanceFrame(character.specialStaticFrames, 0.3f);
            break;
        case Status::SPECIAL_SIDE:
            advanceFrame(character.specialSideFrames, 0.5f);
            break;
        case Status::SPECIAL_UP:
            advanceFrame(character.specialUpFrames, 0.5f);
            break;
        case Status::SPECIAL_DOWN:
            advanceFrame(character.specialDownFrames, 0.21f);
            break;
        case Status::STUNNED:
            advanceFrame(character.stunnedFrames, 0.21f);
            break;
        case Status::DASHING:
            advanceFrame(character.walkFrames, 0.45f);
            break;
        default:
            currentSpriteIndex = 0.0f;
            break;
    }
}

void Player::draw(SDL_Renderer* r, float a) {
    bool flipH = (facing == Facing::LEFT);
    auto drawAnimatedSprite = [&](const std::vector<SDL_Texture*>& frames) {
        if (frames.empty()) return;
        int i = static_cast<int>(currentSpriteIndex) % static_cast<int>(frames.size());
        drawSprite(r, frames[i], flipH, a);
    };

    switch (status) {
        case Status::WALKING:
            drawAnimatedSprite(character.walkFrames);
            break;
        case Status::JUMPING:
            drawAnimatedSprite(character.jumpFrames);
            break;
        case Status::ATTACKING:
            drawAnimatedSprite(character.attackFrames);
            break;
        case Status::SPECIAL_STATIC:
            drawAnimatedSprite(character.specialStaticFrames);
            break;
        case Status::SPECIAL_SIDE:
            drawAnimatedSprite(character.specialSideFrames);
            break;
        case Status::SPECIAL_UP:
            drawAnimatedSprite(character.specialUpFrames);
            break;
        case Status::SPECIAL_DOWN:
            drawAnimatedSprite(character.specialDownFrames);
            break;
        case Status::STUNNED:
            drawAnimatedSprite(character.stunnedFrames);
            break;
        case Status::DASHING:
            drawAnimatedSprite(character.walkFrames);
            break;
        case Status::SHOOTING:
            drawSprite(r, character.shoot, flipH, a);
            break;
        case Status::DAMAGED:
            drawSprite(r, character.damage, flipH, a);
            break;
        case Status::SHIELDED:
            drawSprite(r, character.shielded, flipH, a);
            break;
        default:
            drawSprite(r, character.idle, flipH, a);
            break;
    }

    drawShield(r, a);
    drawShitAura(r, a);
    drawNametag(r, Resources::get().smallFont, a);

    if (grapple) grapple->draw(r, a);
}

void Player::drawShield(SDL_Renderer* r, float a) const {
    if (status != Status::SHIELDED && shieldTimer <= 0.0f) return;
    if (shieldBroken) return;
    float scale = getShieldScale();
    if (scale <= 0.0f) return;

    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    int cx     = drawRect.x + drawRect.w / 2;
    int cy     = drawRect.y + drawRect.h / 2;
    int radius = static_cast<int>((drawRect.w / 2.0f + drawRect.h / 2.0f) / 2 * scale);
    SDL_Rect shieldRect = { cx - radius, cy - radius, 2 * radius, 2 * radius };

    SDL_SetTextureColorMod(shieldTex, color.r, color.g, color.b);
    Renderer::drawSprite(r, shieldTex, &shieldRect, false);
    SDL_SetTextureColorMod(shieldTex, 255, 255, 255);
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
    Renderer::drawHitbox(r, this, a);
    if (grapple) grapple->drawHitbox(r, a);
}

void Player::drawShitAura(SDL_Renderer* r, float a) const {
    if (shitAuraTimer <= 0.0f) return;

    SDL_Rect drawRect = interpolatedRect(prevRect, rect, a);
    int cx     = drawRect.x + drawRect.w / 2;
    int cy     = drawRect.y + drawRect.h / 2;
    float s = shitAuraTimer / SHIT_ITEM_DURATION;
    int radius = static_cast<int>((drawRect.w / 2.0f + drawRect.h / 2.0f) / 2 * s);
    Color c = GREEN; c.a = 128;
    Renderer::fillCircle(r, cx, cy, radius, c);
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