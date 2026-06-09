#include "obj/particle/Particle.h"

#include "core/Resources.h"
#include "misc/Renderer.h"


Particle::Particle(int x, int y, float dx, float dy, const ParticleDefinition& def)
    : Entity({x, y, def.w, def.h}
    , dx, dy
    , Resources::get().getTexture(def.texture))
    , lifetime(def.lifetime)
    , def(def)
{}

void Particle::update(std::vector<std::unique_ptr<Entity>>& entities, float ts) {
    Entity::update(entities, ts);
    dy += def.gravity * ts;
    lifetime -= ts;
}

void Particle::draw(SDL_Renderer* renderer, float a) {
    Renderer::drawEntity(renderer, this, a);
}