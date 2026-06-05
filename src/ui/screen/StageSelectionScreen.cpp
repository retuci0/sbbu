#include "ui/screen/StageSelectionScreen.h"

#include "core/Resources.h"
#include "misc/Common.h"
#include "misc/Renderer.h"
#include "obj/Platform.h"
#include "ui/widget/Button.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <algorithm>
#include <string>


static constexpr int START_BTN_X = 760, START_BTN_Y = 870;
static constexpr int START_BTN_W = 400, START_BTN_H = 80;

StageSelectionScreen::StageSelectionScreen(const Stage& defaultStage, const std::vector<Stage>& stages) : stages(stages)
{
    // find defaultStage in the list, fall back to 0
    for (int i = 0; i < static_cast<int>(stages.size()); ++i) {
        if (stages[i].name == defaultStage.name) {
            selectedIdx = i;
            break;
        }
    }

    addWidget<Button>(START_BTN_X, START_BTN_Y, START_BTN_W, START_BTN_H,
        "select characters", GREEN, WHITE,
        [&]{ confirm(); });

    addWidget<Button>(4, 4, 64, 64, "<", GREEN, WHITE, 
        [&]{ goBack(); });
}

void StageSelectionScreen::navigate(int dir) {
    int n = static_cast<int>(stages.size());
    selectedIdx = (selectedIdx + dir + n) % n;
}

void StageSelectionScreen::confirm() {
    if (stages.empty()) return;

    Stage chosen = stages[selectedIdx];

    // omega: strip all SMALL platforms and grapple points
    if (omega) {
        chosen.platforms.erase(
            std::remove_if(chosen.platforms.begin(), chosen.platforms.end(),
                [](const Platform& p) { return p.size == PlatformSize::SMALL; }),
            chosen.platforms.end());
        chosen.grapplePoints = {};
    }

    result   = { chosen, omega, timeLimit };
    finished = true;
}

void StageSelectionScreen::resetFinished() {
    finished = false;
}

void StageSelectionScreen::handle(const SDL_Event& e) {
    Screen::handle(e);

    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:   navigate(-1); break;
            case SDLK_RIGHT:  navigate(+1); break;

            case SDLK_RETURN: confirm();    break;
            case SDLK_ESCAPE: goBack(); break;

            // tab toggles omega
            case SDLK_TAB:
                omega = !omega;
                break;

            // adjust time limit
            case SDLK_UP:
                if (timeLimit == -1) timeLimit = 60;
                else                 timeLimit = std::min(timeLimit + 30, 600);  // 10 min max
                break;
            case SDLK_DOWN:
                if (timeLimit > 60)  timeLimit -= 30;
                else                 timeLimit = -1;
                break;

            default: break;
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.x >= PREVIEW_X - 60 && e.button.x <= PREVIEW_X - 20
                && e.button.y >= PREVIEW_Y + PREVIEW_H / 2 - 20     
                && e.button.y <= PREVIEW_Y + PREVIEW_H / 2 + 20
        ) {
            navigate(-1);
        }
        if (e.button.x >= PREVIEW_X + PREVIEW_W + 20 && e.button.x <= PREVIEW_X + PREVIEW_W + 60
                && e.button.y >= PREVIEW_Y + PREVIEW_H / 2 - 20     
                && e.button.y <= PREVIEW_Y + PREVIEW_H / 2 + 20
        ) {
            navigate(+1);
        }
    } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
        if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
            omega = !omega;
        }
    }
}

// scales a world rect into the preview box
static SDL_Rect worldToPreview(int wx, int wy, int ww, int wh,
                                int px, int py, int pw, int ph) {
    float sx = static_cast<float>(pw) / SW;
    float sy = static_cast<float>(ph) / SH;
    return {
        px + static_cast<int>(wx * sx),
        py + static_cast<int>(wy * sy),
        std::max(1, static_cast<int>(ww * sx)),
        std::max(1, static_cast<int>(wh * sy)),
    };
}

void StageSelectionScreen::renderPreview(SDL_Renderer* r, const Stage& stage) const {
    const int px = PREVIEW_X, py = PREVIEW_Y;
    const int pw = PREVIEW_W, ph = PREVIEW_H;

    // background
    SDL_Texture* bg = Resources::get().getTexture(stage.bg);
    if (bg) {
        SDL_Rect bgRect = { px, py, pw, ph };
        SDL_RenderCopy(r, bg, nullptr, &bgRect);
    } else {
        Renderer::fillRect(r, px, py, pw, ph, { 30, 30, 50, 255 });
    }

    // thin border
    Renderer::outlineRect(r, px, py, pw, ph, { 200, 200, 200, 180 }, 2);

    // platforms
    for (const auto& plat : stage.platforms) {
        SDL_Rect scaled = worldToPreview(plat.rect.x, plat.rect.y,
                                         plat.rect.w, plat.rect.h,
                                         px, py, pw, ph);

        if (omega && plat.size == PlatformSize::SMALL) continue;
        Renderer::drawSprite(r, plat.image, &scaled, false);
    }

    // grapple points
    if (!omega) {
        for (const auto& gp : stage.grapplePoints) {
            SDL_Rect scaled = worldToPreview(gp.rect.x, gp.rect.y, 
                                             gp.rect.w, gp.rect.h,
                                             px, py, pw, ph);
            Renderer::drawSprite(r, gp.tex, &scaled, false);
        }
    }

    // spawnpoint markers
    for (const auto& sp : stage.spawnpoints) {
        SDL_Rect spRect = worldToPreview(sp.x - 8, sp.y - 8, 16, 16,
                                          px, py, pw, ph);
        int cx = spRect.x + spRect.w / 2;
        int cy = spRect.y + spRect.h / 2;
        Renderer::fillCircle(r, cx, cy, 5, { 100, 200, 255, 200 });
    }
}

void StageSelectionScreen::render(SDL_Renderer* r) {
    // dim background
    Renderer::fillRect(r, 0, 0, SW, SH, {30, 30, 30, 255});

    // title
    Renderer::renderText(r, titleFont, "select stage", SW / 2 - 140, 80, WHITE);

    const Stage& current = stages[selectedIdx];

    renderPreview(r, current);

    std::string label = current.name;
    if (omega) label += " [omega]";
    int tw, th;
    TTF_SizeText(titleFont, label.c_str(), &tw, &th);
    Renderer::renderText(r, titleFont, label,
                         SW / 2 - tw / 2, PREVIEW_Y + PREVIEW_H + 24, WHITE);

    int arrowY = PREVIEW_Y + PREVIEW_H / 2;
    Renderer::renderArrow(r, PREVIEW_X - 40, arrowY, Facing::LEFT);
    Renderer::renderArrow(r, PREVIEW_X + PREVIEW_W + 40, arrowY, Facing::RIGHT);

    // stage dots (pagination indicator)
    int n = static_cast<int>(stages.size());
    int dotsW = n * 18;
    int dotX  = SW / 2 - dotsW / 2;
    int dotY  = PREVIEW_Y + PREVIEW_H + 100;
    for (int i = 0; i < n; ++i) {
        Color c = (i == selectedIdx) ? WHITE : GRAY;
        Renderer::fillCircle(r, dotX + i * 18, dotY, (i == selectedIdx) ? 6 : 4, c);
    }

    // options row
    int optY = dotY + 50;
    Color omegaColor = omega ? Color{100, 255, 100, 255} : GRAY;
    Renderer::renderText(r, font, "[TAB] omega: " + std::string(omega ? "on" : "off"),
                         SW / 2 - 280, optY, omegaColor);

    std::string timeStr = (timeLimit == -1)
                        ? "off"
                        : std::to_string(timeLimit / 60) + ":" +
                          (timeLimit % 60 < 10 ? "0" : "") +
                          std::to_string(timeLimit % 60);
    Renderer::renderText(r, font, "[UP/DOWN] time: " + timeStr,
                         SW / 2 + 20, optY, WHITE);

    // hint
    Renderer::renderText(r, font, "enter to confirm; esc to go back",
                         SW / 2 - 200, SH - 60, GRAY);

    drawWidgets(r, font);
}
