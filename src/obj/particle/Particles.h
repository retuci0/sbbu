#pragma once

#include "obj/particle/Particle.h"


inline constexpr ParticleDefinition DOUBLE_JUMP_PARTICLE = {
    "particle_dj", 18, 18, 18.0f, 0.10f
};

inline constexpr ParticleDefinition DAMAGE_PARTICLE = {
    "particle_damage", 14, 14, 16.0f, 0.12f
};

inline constexpr ParticleDefinition CRIT_PARTICLE = {
    "particle_crit", 22, 22, 24.0f, 0.08f
};

inline constexpr ParticleDefinition DEATH_PARTICLE = {
    "particle_death", 20, 20, 26.0f, 0.06f
};

class DoubleJumpParticle : public Particle {
public:
    DoubleJumpParticle(float x, float y, float dx, float dy)
        : Particle(x, y, dx, dy, DOUBLE_JUMP_PARTICLE) {}
};

class DamageParticle : public Particle {
public:
    DamageParticle(float x, float y, float dx, float dy)
        : Particle(x, y, dx, dy, DAMAGE_PARTICLE) {}
};

class CritParticle : public Particle {
public:
    CritParticle(float x, float y, float dx, float dy)
        : Particle(x, y, dx, dy, CRIT_PARTICLE) {}
};

class DeathParticle : public Particle {
public:
    DeathParticle(float x, float y, float dx, float dy)
        : Particle(x, y, dx, dy, DEATH_PARTICLE) {}
};
