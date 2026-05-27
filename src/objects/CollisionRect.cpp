#include "CollisionRect.h"

#include "../misc/Renderer.h"
#include "../misc/Common.h"


void CollisionRect::drawHitbox(SDL_Renderer* r, float /*a*/) {
    Renderer::outlineRect(r, rect.x, rect.y, rect.w, rect.h, GREEN, 2);
}