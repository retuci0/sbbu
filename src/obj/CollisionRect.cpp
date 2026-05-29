#include "obj/CollisionRect.h"

#include "misc/Renderer.h"
#include "misc/Common.h"


CollisionRect::CollisionRect(int x, int y, int w, int h, Player* owner, int durationFrames)
    : rect({x, y, w, h}), owner(owner), lifetime(durationFrames) {}

void CollisionRect::update(float ts) {
    if (lifetime > 0) lifetime -= ts;
}

void CollisionRect::drawHitbox(SDL_Renderer* r, float /*a*/) {
    Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, GREEN, 2);
}