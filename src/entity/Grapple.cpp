#include "entity/Grapple.h"

#include "entity/GrapplePoint.h"
#include "entity/Player.h"
#include "entity/Platform.h"
#include "entity/Projectile.h"

#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <cmath>
#include <algorithm>


Grapple::Grapple(Player& player, int startX, int startY, float dx, float dy)
    : Entity({ startX, startY, 14, 14 }, dx, dy)
    , owner(player)
{ 
    tex = Resources::get().getTexture("grapple"); 
}


float Grapple::distanceFromOrigin() const {
    float grapplePosX = rect.x + rect.w * 0.5f;
    float grapplePosY = rect.y + rect.h * 0.5f;
    float ownerPosX   = owner.rect.x + owner.rect.w * 0.5f;
    float ownerPosY   = owner.rect.y + owner.rect.h * 0.5f;
    float distX = grapplePosX - ownerPosX;
    float distY = grapplePosY - ownerPosY;
    return std::sqrt(distX * distX + distY * distY);
}

// steers owner toward (hx, hy). returns false when close enough to arrive.
bool Grapple::pullOwnerToward(float hx, float hy, float ts) {
    float px = owner.rect.x + owner.rect.w * 0.5f;
    float py = owner.rect.y + owner.rect.h * 0.5f;

    float toDx = hx - px;
    float toDy = hy - py;
    float dist = std::sqrt(toDx * toDx + toDy * toDy);

    if (dist < ARRIVE_DIST) {
        if (!targetPoint) return false;

        if (targetPoint->type == GrapplePointType::GREEN) {
            // green: kill momentum
            owner.dx = 0.0f;
            owner.dy = 0.0f;
            return false;
        }
        else if (targetPoint->type == GrapplePointType::BLUE) {
            // blue: slingshot
            float pullSpeed = std::sqrt(owner.dx * owner.dx + owner.dy * owner.dy);
            float origSpeed = std::sqrt(playerDx0 * playerDx0 + playerDy0 * playerDy0);
            float launchSpeed = std::max(pullSpeed, origSpeed) + pullSpeed * 0.5f;

            float len = std::sqrt(owner.dx * owner.dx + owner.dy * owner.dy);
            if (len > 0.0f) {
                owner.dx = (owner.dx / len) * launchSpeed;
                owner.dy = (owner.dy / len) * launchSpeed;
            } else {
                owner.dx = playerDx0;
                owner.dy = playerDy0;
            }
            return false;
        }
    }

    // normal pull (for all latched states)
    float invDist = 1.0f / dist;
    float speed   = std::max(std::sqrt(owner.dx * owner.dx + owner.dy * owner.dy),
                             PULL_FORCE);
    owner.dx = toDx * invDist * speed;
    owner.dy = toDy * invDist * speed;
    return true;
}

bool Grapple::pullItemTowardOwner(Item* item, float ts) {
    float px = item->rect.x + item->rect.w / 2.0f;
    float py = item->rect.y + item->rect.h / 2.0f;

    float hx = owner.rect.x + owner.rect.w * 0.5f;
    float hy = owner.rect.y + owner.rect.h * 0.5f;

    float toDx = hx - px;
    float toDy = hy - py;
    float dist = std::sqrt(toDx * toDx + toDy * toDy);

    if (dist < ARRIVE_DIST) return false;

    float invDist = 1.0f / dist;
    float speed   = std::max(std::sqrt(item->dx * item->dx + item->dy * item->dy),
                             PULL_FORCE);

    item->dx = toDx * invDist * speed;
    item->dy = toDy * invDist * speed;

    return true;
}

void Grapple::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    Entity::update(entities, ts);

    if (dx > 0) {
        facing = Facing::RIGHT;
    } else if (dx < 0) {
        facing = Facing::LEFT;
    }

    // reset double jump after latching
    if (isLatched()) owner.hasAirJumped = false;

    switch (state) {

        case GrappleState::FLYING: {
            if (distanceFromOrigin() > MAX_RANGE) {
                retract();
                break;
            }

            for (auto& e : entities) {
                // platforms
                if (Platform* plat = dynamic_cast<Platform*>(e.get())) {
                    if (intersectsWith(*plat)) {
                        dx = dy = 0.0f;
                        state = GrappleState::LATCHED_PLATFORM;
                        alive = true;
                        return;
                    }
                }

                // players (skip self)
                if (Player* player = dynamic_cast<Player*>(e.get())) {                
                    if (player == &owner) continue;
                    if (intersectsWith(*player)) {
                        dx = dy = 0.0f;
                        targetPlayer = player;
                        state = GrappleState::LATCHED_PLAYER;
                        alive = true;
                        return;
                    }
                }

                // projectiles
                if (Projectile* proj = dynamic_cast<Projectile*>(e.get())) {
                    if (intersectsWith(*proj)) {
                        dx = dy = 0.0f;
                        targetProjectile = proj;
                        state = GrappleState::LATCHED_PROJECTILE;
                        alive = true;
                        return;
                    }
                }

                // grapple points
                if (GrapplePoint* point = dynamic_cast<GrapplePoint*>(e.get())) {
                    if (intersectsWith(*point)) {
                        dx = dy = 0.0f;
                        targetPoint = point;
                        state = GrappleState::LATCHED_POINT;
                        alive = true;
                        return;
                    }
                }

                // items (grab towards owner)
                if (Item* item = dynamic_cast<Item*>(e.get())) {
                    if (intersectsWith(*item)) {
                        dx = dy = 0.0f;
                        targetItem = item;
                        state = GrappleState::LATCHED_ITEM;
                        alive = true;
                        return;
                    }
                }
            }

            break;
        }

        case GrappleState::LATCHED_PLATFORM: {
            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) {
                alive = false;
                return;
            }
            break;
        }

        case GrappleState::LATCHED_PLAYER: {
            // target was destroyed externally
            if (!targetPlayer) {
                retract();
                break;
            }
            // hook tracks the target player's center
            rect.x = targetPlayer->rect.x + (targetPlayer->rect.w - rect.w) / 2;
            rect.y = targetPlayer->rect.y + (targetPlayer->rect.h - rect.h) / 2;

            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) {
                alive = false;
                return;
            }
            break;
        }

        case GrappleState::LATCHED_PROJECTILE: {
            if (!targetProjectile) {
                retract();
                break;
            }
            // hook tracks the projectile
            rect.x = targetProjectile->rect.x + (targetProjectile->rect.w - rect.w) / 2;
            rect.y = targetProjectile->rect.y + (targetProjectile->rect.h - rect.h) / 2;

            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) {
                alive = false;
                return;
            }
            break;
        }

        case GrappleState::LATCHED_POINT: {
            if (!targetPoint) {
                retract();
                break;
            }
            // snapshot the player's velocity exactly once when we first latch
            if (targetPoint->type == GrapplePointType::BLUE && !velocitySnapshotted) {
                playerDx0 = owner.dx;
                playerDy0 = owner.dy;
                velocitySnapshotted = true;
            }

            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) {
                alive = false;
                return;
            }
            break;
        }

        case GrappleState::LATCHED_ITEM: {
            if (!targetItem) {
                retract();
                break;
            }

            rect.x = targetItem->rect.x + (targetItem->rect.w - rect.w) / 2;
            rect.y = targetItem->rect.y + (targetItem->rect.h - rect.h) / 2;

            if (distanceFromOrigin() > MAX_RANGE) {
                SDL_Log("retracting");
                retract();
                break;
            }

            if (!pullItemTowardOwner(targetItem, ts)) {
                alive = false;
                return;
            }
            break;
        }

        case GrappleState::RETRACTING: {
            float px = owner.rect.x + owner.rect.w * 0.5f;
            float py = owner.rect.y + owner.rect.h * 0.5f;
            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;

            float toDx = px - hx;
            float toDy = py - hy;
            float dist = std::sqrt(toDx * toDx + toDy * toDy);

            if (dist < RETRACT_SPEED * ts) {
                alive = false;
                return;
            }

            float invDist = 1.0f / dist;
            dx = static_cast<int>(toDx * invDist * RETRACT_SPEED);
            dy = static_cast<int>(toDy * invDist * RETRACT_SPEED);
            break;
        }
    }
}


void Grapple::draw(SDL_Renderer* r, float a) {
    SDL_Rect drawRect = interpolatedRect(a);
    SDL_Rect playerRect = owner.interpolatedRect(a);
    // rope from player center to hook center
    int px = playerRect.x + playerRect.w / 2;
    int py = playerRect.y + playerRect.h / 2;
    int hx = drawRect.x + drawRect.w / 2;
    int hy = drawRect.y + drawRect.h / 2;

    Color c;
    // hook rope color by state
    switch (state) {
        case GrappleState::LATCHED_PLATFORM:   c = GRAY;   break;
        case GrappleState::LATCHED_PLAYER:     c = RED;    break;
        case GrappleState::LATCHED_PROJECTILE: c = YELLOW; break;
        case GrappleState::LATCHED_POINT:      c = targetPoint->type == GrapplePointType::BLUE
                                                                      ? BLUE : LIME;    break;
        case GrappleState::LATCHED_ITEM:       c = LIME;   break;
        default:                               c = CYAN;   break;
    }

    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 180);

    // draw three lines to emulate a thick line
    SDL_RenderDrawLine(r, px, py, hx, hy);
    SDL_RenderDrawLine(r, px, py - 1, hx, hy - 1);
    SDL_RenderDrawLine(r, px, py + 1, hx, hy + 1);

    Renderer::drawEntity(r, this, a);
}

void Grapple::drawHitbox(SDL_Renderer* r, float a) const {
    Renderer::drawHitbox(r, this, a);
}


bool Grapple::isLatched() const {
    return state == GrappleState::LATCHED_PLATFORM
        || state == GrappleState::LATCHED_PLAYER
        || state == GrappleState::LATCHED_PROJECTILE
        || state == GrappleState::LATCHED_POINT
        || state == GrappleState::LATCHED_ITEM;
}

void Grapple::retract() {
    state = GrappleState::RETRACTING;
    dx = dy = 0;
    playerDx0           = 0;
    playerDy0           = 0;
    velocitySnapshotted = false;
    targetPlayer        = nullptr;
    targetProjectile    = nullptr;
    targetPoint         = nullptr;
    targetItem          = nullptr;
}