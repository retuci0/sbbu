#pragma once

#include <SDL2/SDL_render.h>


class Particle {
public:
    virtual ~Particle() = default;

    virtual void update(float ts);
    virtual void draw(SDL_Renderer* renderer, float a) const = 0;

    bool isAlive() const { return lifetime > 0.0f; }

protected:
    Particle(float x, float y, float vx, float vy, int w, int h, float lifetime);

    float x = 0.0f, y = 0.0f;
    float prevX = 0.0f, prevY = 0.0f;
    float vx = 0.0f, vy = 0.0f;
    int w = 0, h = 0;
    float lifetime = 0.0f;
};

struct ParticleDefinition {
    const char* textureName;
    int w;
    int h;
    float lifetime;
    float gravity;
};

class TextureParticle : public Particle {
public:
    TextureParticle(const ParticleDefinition& definition, float x, float y, float vx, float vy);

    void update(float ts) override;
    void draw(SDL_Renderer* renderer, float a) const override;

protected:
    const ParticleDefinition& definition;
    SDL_Texture* texture = nullptr;
};
