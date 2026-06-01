#pragma once

#include "obj/Platform.h"

#include <SDL2/SDL_render.h>

#include <string>
#include <vector>


struct Spawnpoint {
    int x, y;
};

struct Stage {
    std::string name;
    std::string bg;  // key into Resources::getTexture()
    std::vector<Platform> platforms;
    std::vector<Spawnpoint> spawnpoints;
};

extern Stage classicStage();
extern Stage pillarStage();
extern Stage flatStage();

extern std::vector<Stage> allStages();