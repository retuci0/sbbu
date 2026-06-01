#include "obj/particle/Particle.h"

#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"


Particle::Particle(float x, float y, float vx, float vy, int w, int h, float lifetime)
    : x(x), y(y), prevX(x), prevY(y), vx(vx), vy(vy), w(w), h(h), lifetime(lifetime)
{
}

void Particle::update(float ts) {
    prevX = x;
    prevY = y;
    x += vx * ts;
    y += vy * ts;
    lifetime -= ts;
}

TextureParticle::TextureParticle(const ParticleDefinition& definition, float x, float y, float vx, float vy)
    : Particle(x - definition.w / 2.0f, y - definition.h / 2.0f,
               vx, vy, definition.w, definition.h, definition.lifetime),
      definition(definition),
      texture(Resources::get().getTexture(definition.textureName))
{
}

void TextureParticle::update(float ts) {
    vy += definition.gravity * ts;
    Particle::update(ts);
}

void TextureParticle::draw(SDL_Renderer* renderer, float a) const {
    SDL_Rect prev = { static_cast<int>(prevX), static_cast<int>(prevY), w, h };
    SDL_Rect current = { static_cast<int>(x), static_cast<int>(y), w, h };
    SDL_Rect rect = interpolatedRect(prev, current, a);
    Renderer::drawSprite(renderer, texture, &rect, false);
}
