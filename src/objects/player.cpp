#include "player.h"

#include "platform.h"
#include "../misc/common.h"

#include <algorithm>


// how many pixels of platform-top overlap counts as an edge to climb (instead of wall)
static constexpr int EDGE_CLIMB_THRESHOLD = 12;


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

    int w = 64, h = 64;
    if (ch->idle) {
        SDL_QueryTexture(ch->idle, nullptr, nullptr, &w, &h);
    }
    rect = {x, y, w, h};
}

void Player::move(int direction) {
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
    if (damageSound) { Mix_PlayChannel(-1, damageSound, 0); }
    status = Status::DAMAGED;

    float damageTaken = static_cast<float>(character->stats.health - hp);
    float w = character->stats.weight;

    if (side == Facing::LEFT) {
        dx = -damageTaken * w;
    } else {
        dx = damageTaken * w;
    }
    dy = -1.0f * (damageTaken / 10.0f) * w;
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
                if (dx > 0) { rect.x = p.rect.x - rect.w; }
                if (dx < 0) { rect.x = p.rect.x + p.rect.w; }
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

    // sync status
    if (status != Status::DAMAGED && status != Status::ATTACKING) {
        if (!onGround) {
            status = Status::JUMPING;
        } else if (dx != 0.f) {
            status = Status::WALKING;
        } else {
            status = Status::IDLE;
        }
    }

    animate();
}

void Player::drawSprite(SDL_Renderer* r, SDL_Texture* tex, bool flipH) const {
    if (!tex) { return; }
    SDL_Rect dst = {rect.x, rect.y, rect.w, rect.h};
    SDL_RendererFlip flip = flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
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
        default:
            currentSpriteIndex = 0.0f;
            break;
    }
}

void Player::draw(SDL_Renderer* r) const {
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
            drawSprite(r, character->attack, flipH);
            break;
        case Status::DAMAGED:
            drawSprite(r, character->damage, flipH);
            break;
        case Status::IDLE:
        default:
            drawSprite(r, character->idle, flipH);
            break;
    }

    // colored indicator above the player's head so each player is easy to tell apart
    fillRect(r,
        rect.x + rect.w / 2 - 8, rect.y - 10,
        16, 5,
        color.r, color.g, color.b, color.a);
}

void Player::drawHitboxes(SDL_Renderer* r) const {
    outlineRect(r, rect.x, rect.y, rect.w, rect.h, RED);
}