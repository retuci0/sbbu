#include "obj/particle/ParticleManager.h"

#include "obj/particle/Particles.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>


namespace {
    constexpr float PI = 3.14159264f;

    float randomFloat(float min, float max) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    template <typename P>
    void spawnBurst(std::vector<std::unique_ptr<Particle>>& particles, float x, float y, int count, float minSpeed, float maxSpeed, float upwardBias) {
        particles.reserve(particles.size() + count);
        for (int i = 0; i < count; ++i) {
            float angle = randomFloat(0.0f, PI * 2.0f);
            float speed = randomFloat(minSpeed, maxSpeed);
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed - upwardBias;
            particles.push_back(std::make_unique<P>(x, y, vx, vy));
        }
    }
}

void ParticleManager::update(float ts) {
    for (auto& particle : particles) {
        particle->update(ts);
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const auto& particle) { return !particle->isAlive(); }),
        particles.end());
}

void ParticleManager::draw(SDL_Renderer* renderer, float a) const {
    for (const auto& particle : particles) {
        particle->draw(renderer, a);
    }
}

void ParticleManager::clear() {
    particles.clear();
}


/* methods for spawning particles */

void ParticleManager::spawnDoubleJump(int x, int y) {
    if (!*particlesEnabled) return;
    spawnBurst<DoubleJumpParticle>(particles, x, y, 10, 0.8f, 2.2f, -0.8f);
}

void ParticleManager::spawnDamage(int x, int y) {
    if (!*particlesEnabled) return;
    spawnBurst<DamageParticle>(particles, x, y, 8, 0.9f, 2.8f, 0.6f);
}

void ParticleManager::spawnCrit(int x, int y) {
    if (!*particlesEnabled) return;
    spawnBurst<CritParticle>(particles, x, y, 14, 1.2f, 3.8f, 1.2f);
}

void ParticleManager::spawnDeath(int x, int y) {
    if (!*particlesEnabled) return;
    spawnBurst<DeathParticle>(particles, x, y, 18, 1.0f, 3.2f, 0.4f);
}
