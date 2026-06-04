#include "obj/Grapple.h"
#include "core/Resources.h"
#include "obj/Player.h"
#include "obj/Platform.h"
#include "obj/Projectile.h"
#include "misc/Common.h"
#include "misc/Renderer.h"

#include <SDL2/SDL_rect.h>
#include <cmath>
#include <algorithm>


Grapple::Grapple(Player& player, int startX, int startY, float velX, float velY)
    : rect({ startX, startY, 14, 14 })
    , dx(velX)
    , dy(velY)
    , owner(player)
    , originX(static_cast<float>(startX))
    , originY(static_cast<float>(startY))
{}


float Grapple::distanceFromOrigin() const {
    float ex = rect.x - originX;
    float ey = rect.y - originY;
    return std::sqrt(ex * ex + ey * ey);
}

// steers owner toward (hx, hy). returns false when close enough to arrive.
bool Grapple::pullOwnerToward(float hx, float hy, float ts) {
    float px = owner.rect.x + owner.rect.w * 0.5f;
    float py = owner.rect.y + owner.rect.h * 0.5f;

    float toDx = hx - px;
    float toDy = hy - py;
    float dist = std::sqrt(toDx * toDx + toDy * toDy);

    if (dist < ARRIVE_DIST) return false;

    float invDist = 1.0f / dist;
    float speed   = std::max(std::sqrt(owner.dx * owner.dx + owner.dy * owner.dy),
                             PULL_FORCE);
    owner.dx = toDx * invDist * speed;
    owner.dy = toDy * invDist * speed;
    return true;
}


bool Grapple::update(const std::vector<Platform>& platforms,
                     std::vector<Player>&         players,
                     std::vector<Projectile>&     projectiles,
                     float ts)
{
    switch (state) {

        case GrappleState::FLYING: {
            rect.x += static_cast<int>(dx * ts);
            rect.y += static_cast<int>(dy * ts);

            if (distanceFromOrigin() > MAX_RANGE) {
                state = GrappleState::RETRACTING;
                break;
            }

            // platforms first
            for (const auto& p : platforms) {
                if (SDL_HasIntersection(&p.rect, &rect)) {
                    dx = dy = 0.0f;
                    state = GrappleState::LATCHED_PLATFORM;
                    return true;
                }
            }

            // players (skip self)
            for (auto& p : players) {
                if (&p == &owner) continue;
                if (SDL_HasIntersection(&p.rect, &rect)) {
                    dx = dy = 0.0f;
                    targetPlayer = &p;
                    state = GrappleState::LATCHED_PLAYER;
                    return true;
                }
            }

            // projectiles
            for (auto& proj : projectiles) {
                if (SDL_HasIntersection(&proj.rect, &rect)) {
                    dx = dy = 0.0f;
                    targetProjectile = &proj;
                    state = GrappleState::LATCHED_PROJECTILE;
                    return true;
                }
            }

            break;
        }

        case GrappleState::LATCHED_PLATFORM: {
            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) return false;
            break;
        }

        case GrappleState::LATCHED_PLAYER: {
            if (!targetPlayer) {
                // target was destroyed externally
                state = GrappleState::RETRACTING;
                break;
            }
            // hook tracks the target player's center
            rect.x = targetPlayer->rect.x + (targetPlayer->rect.w - rect.w) / 2;
            rect.y = targetPlayer->rect.y + (targetPlayer->rect.h - rect.h) / 2;

            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) return false;
            break;
        }

        case GrappleState::LATCHED_PROJECTILE: {
            if (!targetProjectile) {
                state = GrappleState::RETRACTING;
                break;
            }
            // hook tracks the projectile
            rect.x = targetProjectile->rect.x + (targetProjectile->rect.w - rect.w) / 2;
            rect.y = targetProjectile->rect.y + (targetProjectile->rect.h - rect.h) / 2;

            float hx = rect.x + rect.w * 0.5f;
            float hy = rect.y + rect.h * 0.5f;
            if (!pullOwnerToward(hx, hy, ts)) return false;
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

            if (dist < RETRACT_SPEED * ts) return false;

            float invDist = 1.0f / dist;
            rect.x += static_cast<int>(toDx * invDist * RETRACT_SPEED * ts);
            rect.y += static_cast<int>(toDy * invDist * RETRACT_SPEED * ts);
            break;
        }
    }

    return true;
}


void Grapple::draw(SDL_Renderer* r) const {
    // rope from player center to hook center
    int px = owner.rect.x + owner.rect.w / 2;
    int py = owner.rect.y + owner.rect.h / 2;
    int hx = rect.x + rect.w / 2;
    int hy = rect.y + rect.h / 2;

    Color c;
    // hook rope color by state
    switch (state) {
        case GrappleState::LATCHED_PLATFORM:   c = GREEN;  break;
        case GrappleState::LATCHED_PLAYER:     c = RED;    break;
        case GrappleState::LATCHED_PROJECTILE: c = YELLOW; break;
        default:                               c = CYAN;   break;
    }

    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 180);

    // draw three lines to emulate a thick line
    SDL_RenderDrawLine(r, px, py, hx, hy);
    SDL_RenderDrawLine(r, px, py - 1, hx, hy - 1);
    SDL_RenderDrawLine(r, px, py + 1, hx, hy + 1);

    Renderer::drawSprite(r, Resources::get().getTexture("grapple"), &rect, dx < 0);
    // Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, hookColor, 2);
}


bool Grapple::isLatched() const {
    return state == GrappleState::LATCHED_PLATFORM
        || state == GrappleState::LATCHED_PLAYER
        || state == GrappleState::LATCHED_PROJECTILE;
}

void Grapple::retract() {
    state = GrappleState::RETRACTING;
}