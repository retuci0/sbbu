#include "projectile.h"

#include "../misc/common.h"


Projectile::Projectile(SDL_Texture* imgLeft, SDL_Texture* imgRight, int x, int y, Facing dir)
    : direction(dir), img_left(imgLeft), img_right(imgRight)
{
    rect = {x, y, 64, 64};
}

void Projectile::move() {
    if (direction == Facing::LEFT) rect.x -= static_cast<int>(velocity);
    else rect.x += static_cast<int>(velocity);
}

void Projectile::draw(SDL_Renderer* r) {
    move();
    SDL_Texture* tex = (direction == Facing::LEFT) ? img_left : img_right;
    if (tex) SDL_RenderCopy(r, tex, nullptr, &rect);
}

void Projectile::drawHitboxes(SDL_Renderer* r) const {
    outlineRect(r, rect.x, rect.y, rect.w, rect.h, GREEN);
}