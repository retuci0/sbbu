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

class DoubleJumpParticle : public TextureParticle {
public:
    DoubleJumpParticle(float x, float y, float vx, float vy)
        : TextureParticle(DOUBLE_JUMP_PARTICLE, x, y, vx, vy) {}
};

class DamageParticle : public TextureParticle {
public:
    DamageParticle(float x, float y, float vx, float vy)
        : TextureParticle(DAMAGE_PARTICLE, x, y, vx, vy) {}
};

class CritParticle : public TextureParticle {
public:
    CritParticle(float x, float y, float vx, float vy)
        : TextureParticle(CRIT_PARTICLE, x, y, vx, vy) {}
};

class DeathParticle : public TextureParticle {
public:
    DeathParticle(float x, float y, float vx, float vy)
        : TextureParticle(DEATH_PARTICLE, x, y, vx, vy) {}
};
