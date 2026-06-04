#include "Stages.h"


Stage classicStage() {
    Stage s;
    s.name = "classic";
    s.bg = "bg_classic";
    s.platforms = {
        Platform(360,  500, 1200, 300, PlatformSize::BIG),
        Platform(640,  250,  200,  30, PlatformSize::SMALL),
        Platform(1080, 250,  200,  30, PlatformSize::SMALL),
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
        Platform(1220, 200,  200,  30, PlatformSize::SMALL),
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
        Platform(1320,320,  200,  30, PlatformSize::SMALL),
    };
    s.spawnpoints = { {500, 0}, {1420, 0} };
    s.deathZones = { -500, SW + 500, SH + 100, -1000 };
    return s;
}

Stage dashStage() {
    Stage s;
    s.name = "dash";
    s.bg = "bg_dash";
    s.platforms = {
        Platform(410, 550, 1100, 250, PlatformSize::BIG),
        Platform(340, 380, 200, 30, PlatformSize::SMALL),
        Platform(340, 220, 200, 30, PlatformSize::SMALL),
        Platform(1380, 380, 200, 30, PlatformSize::SMALL),
        Platform(1380, 220, 200, 30, PlatformSize::SMALL),
    };
    s.spawnpoints = { {440, 0}, {1480, 0} };
    s.deathZones = { -500, SW + 500, SH - 200, -1000 };
    return s;
}

std::vector<Stage> allStages() {
    return { classicStage(), pillarStage(), flatStage(), dashStage() };
}