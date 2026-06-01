#pragma once

#include "obj/particle/Particle.h"

#include <SDL2/SDL_render.h>

#include <memory>
#include <vector>


class ParticleManager {
public:
    ParticleManager(bool* particlesEnabled) : particlesEnabled(particlesEnabled) {}

    void update(float ts);
    void draw(SDL_Renderer* renderer, float a) const;
    void clear();

    void spawnDoubleJump(int x, int y);
    void spawnDamage(int x, int y);
    void spawnCrit(int x, int y);
    void spawnDeath(int x, int y);

private:
    bool* particlesEnabled = nullptr;
    std::vector<std::unique_ptr<Particle>> particles;
};
