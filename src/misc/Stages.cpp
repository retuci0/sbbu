#include "Stages.h"

#include "entity/GrapplePoint.h"
#include "entity/Platform.h"
#include <random>
#include <vector>


void disableRandomPlatform(std::vector<std::unique_ptr<Entity>> &entities) {
    static std::mt19937 rng(std::random_device{}());
    std::vector<Platform*> platforms = {};
    for (auto& e : entities) {
        if (Platform* p = dynamic_cast<Platform*>(e.get())) {
            if (p->active) platforms.push_back(p);
        }
    }
    if (platforms.empty()) return;
    std::uniform_int_distribution<int> platPick(0, static_cast<int>(platforms.size()) - 1);
    Platform* plat = platforms[platPick(rng)];

    plat->inactiveTimer = 180;
}

Stage classicStage() {
    Stage s;
    s.name = "classic";
    s.bg = "bg_classic";
    s.platforms = {
        Platform(360,  500, 1200, 300, PlatformSize::BIG),
        Platform(640,  250,  200,  30, PlatformSize::SMALL),
        Platform(1080, 250,  200,  30, PlatformSize::SMALL)
    };
    s.spawnpoints = { {740, 0}, {1180, 0} };
    s.deathZones = { -500, SW + 500, SH + 100, -1000 };
    return s;
}

Stage pillarStage() {
    Stage s;
    s.name = "pillar";
    s.bg = "bg_pillar";
    s.platforms = {
        Platform(200,  600,  500, 200, PlatformSize::BIG),
        Platform(1220, 600,  500, 200, PlatformSize::BIG),
        Platform(760,  350,  400,  30, PlatformSize::SMALL),
        Platform(500,  200,  200,  30, PlatformSize::SMALL),
        Platform(1220, 200,  200,  30, PlatformSize::SMALL)
    };
    s.spawnpoints = { {450, 0}, {1470, 0} };
    s.deathZones = { -500, SW + 500, SH + 100, -1000 };
    return s;
}

Stage flatStage() {
    Stage s;
    s.name = "flatzone";
    s.bg = "bg_flat";
    s.platforms = {
        Platform(160, 550, 1600, 250, PlatformSize::BIG),
        Platform(400, 320,  200,  30, PlatformSize::SMALL),
        Platform(860, 220,  200,  30, PlatformSize::SMALL),
        Platform(1320,320,  200,  30, PlatformSize::SMALL)
    };
    s.spawnpoints = { {500, 0}, {1420, 0} };
    s.deathZones = { -500, SW + 500, SH + 100, -1000 };
    s.grapplePoints = {
        GrapplePoint(GrapplePointType::GREEN, SDL_Rect{ 100, 100, 56, 56 }),
        GrapplePoint(GrapplePointType::GREEN, SDL_Rect{ SW - 156, 100, 56, 56 })
    };
    return s;
}

Stage dashStage() {
    Stage s;
    s.name = "dash";
    s.bg = "bg_dash";
    s.platforms = {
        Platform(410, 550, 1100, 250, PlatformSize::BIG),
        Platform(340, 380,  200,  30, PlatformSize::SMALL),
        Platform(340, 220,  200,  30, PlatformSize::SMALL),
        Platform(1380,380,  200,  30, PlatformSize::SMALL),
        Platform(1380,220,  200,  30, PlatformSize::SMALL)
    };
    s.spawnpoints = { {440, 0}, {1480, 0} };
    s.deathZones = { -500, SW + 500, SH - 200, -1000 };
    s.grapplePoints = {
        GrapplePoint(GrapplePointType::BLUE, SDL_Rect{ (SW - 56) / 2, 300, 56, 56 })
    };
    return s;
}

Stage hellStage() {
    Stage s;
    s.name = "hell";
    s.bg = "bg_hell";
    s.platforms = {
        Platform(410, 200, 1100, 250, PlatformSize::BIG),
        Platform(340, 550,  200,  30, PlatformSize::SMALL),
        Platform(1380,550,  200,  30, PlatformSize::SMALL)
    };
    s.spawnpoints = { {440, 400}, {1480, 400} };
    s.deathZones = { -500, SW + 500, SH + 300, -2000 };
    s.grapplePoints = {
        GrapplePoint(GrapplePointType::BLUE, SDL_Rect{ (SW - 56) / 2, 600, 56, 56 }),
        GrapplePoint(GrapplePointType::BLUE, SDL_Rect{ (SW - 56) / 2, 500, 56, 56 }),
        GrapplePoint(GrapplePointType::YELLOW, SDL_Rect{ 180, 480, 56, 56 }, disableRandomPlatform, 180),
        GrapplePoint(GrapplePointType::YELLOW, SDL_Rect{ SW - 180 - 56, 480, 56, 56 }, disableRandomPlatform, 180)
    };
    return s;
}

std::vector<Stage> allStages() {
    return { classicStage(), pillarStage(), flatStage(), dashStage(), hellStage() };
}