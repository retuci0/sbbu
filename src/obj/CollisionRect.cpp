#include "obj/CollisionRect.h"

#include "misc/Renderer.h"


CollisionRect::CollisionRect(int x, int y, int w, int h, Player* owner, int durationFrames)
    : Entity({ x, y, w, h }), owner(owner), lifetime(durationFrames) {}

void CollisionRect::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    Entity::update(entities, ts);
    if (lifetime > 0) lifetime -= ts;
}

void CollisionRect::drawHitbox(SDL_Renderer* r, float /*a*/) {
    Renderer::drawHitbox(r, this, -1);
}