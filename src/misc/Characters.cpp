#include "misc/Characters.h"
#include "entity/Player.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>

#include <string>


//////////////////////////////////////
/*         DEFAULT SPECIALS         */
//////////////////////////////////////

SpecialHitboxParams Character::specialStatic(Player* player) const {
    int hw = static_cast<int>(110 * player->scale);
    int hh = player->rect.h + static_cast<int>(20 * player->scale);
    int hx = (player->facing == Facing::RIGHT) ? player->rect.x + player->rect.w - static_cast<int>(20 * player->scale)
                                              : player->rect.x - hw + static_cast<int>(20 * player->scale);
    int hy = player->rect.y - static_cast<int>(10 * player->scale);
    return { hx, hy, hw, hh, 3.0f, 5.0f };
}

SpecialHitboxParams Character::specialSide(Player* player) const {
    int hw = static_cast<int>(130 * player->scale);
    int hh = player->rect.h;
    int hx = (player->facing == Facing::RIGHT) ? player->rect.x + player->rect.w - static_cast<int>(30 * player->scale)
                                              : player->rect.x - hw + static_cast<int>(30 * player->scale);
    int hy = player->rect.y;
    return { hx, hy, hw, hh, 2.5f, 4.0f };
}

SpecialHitboxParams Character::specialUp(Player* player) const {
    int hw = player->rect.w + static_cast<int>(20 * player->scale);
    int hh = static_cast<int>(90 * player->scale);
    int hx = player->rect.x - static_cast<int>(10 * player->scale);
    int hy = player->rect.y - hh + static_cast<int>(20 * player->scale);
    return { hx, hy, hw, hh, 3.5f, 6.0f };
}

SpecialHitboxParams Character::specialDown(Player* player) const {
    int hw = player->rect.w + static_cast<int>(40 * player->scale);
    int hh = static_cast<int>(80 * player->scale);
    int hx = player->rect.x - static_cast<int>(20 * player->scale);
    int hy = player->rect.y + player->rect.h - static_cast<int>(20 * player->scale);
    return { hx, hy, hw, hh, 4.0f, 7.0f, true };
}

void Character::onSpecialStatic(Player* player) const {
    player->dx = 0.0f;
    player->dy = 0.0f;
}

void Character::onSpecialSide(Player* player) const {
    player->dx = (player->facing == Facing::RIGHT) ? 12.0f : -12.0f;
    player->dy = 0.0f;
}

void Character::onSpecialUp(Player* player) const {
    player->dx = 0.0f;
    player->dy = -14.0f;
}

void Character::onSpecialDown(Player* player) const {
    player->dx = 0.0f;
    player->dy = 16.0f;
}


//////////////////////////////////////
/*        CHARACTER SUBCLASSES      */
//////////////////////////////////////

BertCharacter::BertCharacter() {
    stats = { "Bert", "average human.",
              100, 5, 3, 5.0f, 12.2f, 0.20f, 0.50f, 20.0f, 1.0f };
}

BerrotaCharacter::BerrotaCharacter() {
    stats = { "Berrota", "lightweight skinny mf, moves faster than your mom",
              90, 5, 3, 7.0f, 13.2f, 0.35f, 0.50f, 17.0f, 1.5f };
}

LorcCharacter::LorcCharacter() {
    stats = { "Lorc", "heavy tank - only weakness is gravity",
              200, 5, 3, 3.0f, 7.5f, 0.10f, 0.72f, 25.0f, 0.5f };
}

// lorc slams harder and wider
SpecialHitboxParams LorcCharacter::specialDown(Player* player) const {
    int hw = player->rect.w + static_cast<int>(80 * player->scale);
    int hh = static_cast<int>(100 * player->scale);
    int hx = player->rect.x - static_cast<int>(40 * player->scale);
    int hy = player->rect.y + player->rect.h - static_cast<int>(20 * player->scale);
    return { hx, hy, hw, hh, 6.0f, 10.0f, true };
}

void LorcCharacter::onSpecialDown(Player* player) const {
    player->dx = 0.0f;
    player->dy = 22.0f;  // slams faster
}

JordiCharacter::JordiCharacter() {
    stats = { "Jordi", "strong but fragile",
              80, 10, 5, 5.1f, 11.0f, 0.30f, 0.50f, 17.0f, 1.2f };
}

BarcosCharacter::BarcosCharacter() {
    stats = { "Barcos", "small ahh mf",
              50, 5, 3, 12.0f, 11.0f, 0.5f, 0.5f, 17.0f, 2.0f };
}

AlsexitoCharacter::AlsexitoCharacter() {
    stats = { "Alsexito", "your average gay gymbro",
              120, 20, 2, 2.5f, 2.5f, 0.8f, 0.72f, 25.0f, 2.0f };
}

FlanCharacter::FlanCharacter() {
    stats = { "Flan", "fran-chan tarro-ponce",
              100, 5, 5, 6.0f, 16.0f, 0.8f, 0.5f, 17.0f, 1.0f };
}


//////////////////////////////////////
/*          ASSET LOADING           */
//////////////////////////////////////

static SDL_Texture* loadTex(SDL_Renderer* r, const std::string& path) {
    SDL_Texture* t = IMG_LoadTexture(r, path.c_str());
    if (!t) {
        SDL_Log("loadCharacterAssets: failed to load \"%s\": %s", path.c_str(), IMG_GetError());
    }
    return t;
}

void Character::unload() {
    auto destroyAnim = [](std::vector<SDL_Texture*>& a) {
        for (SDL_Texture* t : a) if (t) SDL_DestroyTexture(t);
        a.clear();
    };
    auto destroyFrame = [](SDL_Texture*& t) {
        if (t) SDL_DestroyTexture(t);
        t = nullptr;
    };

    destroyAnim(walkFrames);
    destroyAnim(jumpFrames);
    destroyAnim(attackFrames);
    destroyAnim(specialStaticFrames);
    destroyAnim(specialSideFrames);
    destroyAnim(specialUpFrames);
    destroyAnim(specialDownFrames);
    destroyAnim(stunnedFrames);

    destroyFrame(idle);
    destroyFrame(shoot);
    destroyFrame(damage);
    destroyFrame(shielded);
    destroyFrame(icon);
    destroyFrame(deadIcon);

    loaded = false;
}

void loadCharacter(Character& c, SDL_Renderer* r, const std::string& folder) {
    if (c.loaded) return;

    for (int i = 0; i < 3; ++i) {
        c.stunnedFrames.push_back(loadTex(r, folder + "/stunned/" + std::to_string(i) + ".png"));
        c.specialDownFrames.push_back(loadTex(r, folder + "/special/down/" + std::to_string(i) + ".png"));
    }
    for (int i = 0; i < 5; ++i) {
        c.walkFrames.push_back(loadTex(r, folder + "/walk/" + std::to_string(i) + ".png"));
        c.jumpFrames.push_back(loadTex(r, folder + "/jump/" + std::to_string(i) + ".png"));
        c.attackFrames.push_back(loadTex(r, folder + "/attack/" + std::to_string(i) + ".png"));
        c.specialSideFrames.push_back(loadTex(r, folder + "/special/side/" + std::to_string(i) + ".png"));
        c.specialUpFrames.push_back(loadTex(r, folder + "/special/up/" + std::to_string(i) + ".png"));
    }
    for (int i = 0; i < 6; ++i) {
        c.specialStaticFrames.push_back(loadTex(r, folder + "/special/static/" + std::to_string(i) + ".png"));
    }

    c.idle     = loadTex(r, folder + "/idle/0.png");
    c.shoot    = loadTex(r, folder + "/shoot/0.png");
    c.damage   = loadTex(r, folder + "/damage/0.png");
    c.shielded = loadTex(r, folder + "/shield/0.png");

    c.icon     = loadTex(r, folder + "/icon.png");
    c.deadIcon = loadTex(r, folder + "/icon_dead.png");

    c.loaded = true;
}