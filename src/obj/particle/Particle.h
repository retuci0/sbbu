#pragma once

#include "obj/Entity.h"
#include <SDL2/SDL_render.h>
#include <memory>


struct ParticleDefinition {
    const char* texture;
    int w, h;
    float lifetime;
    float gravity;
};

class Particle : public Entity {
public:
    Particle(int x, int y, float dx, float dy, const ParticleDefinition& def);
    virtual ~Particle() = default;

    virtual void update(std::vector<std::unique_ptr<Entity>>& entities, float ts) override;
    virtual void draw(SDL_Renderer* renderer, float a) override;

    bool isAlive() const { return lifetime > 0.0f; }

    EntityType getType() const override {
        return EntityType::MISC;
    }

protected:
    Particle(int x, int y, int w, int h, float dx, float dy, float lifetime);

    float dx = 0.0f, dy = 0.0f;
    float lifetime = 0.0f;

    const ParticleDefinition& def;
};
