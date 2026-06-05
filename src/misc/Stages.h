#pragma once

#include "obj/GrapplePoint.h"
#include "obj/Platform.h"
#include "obj/Player.h"

#include <SDL2/SDL_render.h>

#include <string>
#include <vector>


struct Spawnpoint {
    int x, y;
};

struct DeathZones {
    int leftX, rightX;
    int bottomY, topY;
};

struct Stage {
    std::string name;
    std::string bg;  // key into Resources::getTexture()
    std::vector<Platform> platforms;
    std::vector<GrapplePoint> grapplePoints;
    std::vector<Spawnpoint> spawnpoints;
    DeathZones deathZones;

    bool isOutsideWorld(const Player& p) const {
        return p.rect.x >= deathZones.rightX
            || p.rect.x <= deathZones.leftX
            || p.rect.y >= deathZones.bottomY
            || p.rect.y <= deathZones.topY;
    }
};

extern Stage classicStage();
extern Stage pillarStage();
extern Stage flatStage();
extern Stage dashStage();
extern Stage hellStage();

extern std::vector<Stage> allStages();