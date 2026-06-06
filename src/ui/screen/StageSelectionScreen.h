#pragma once

#include "misc/Stages.h"
#include "ui/Screen.h"

#include <vector>


struct StageSelectionResult {
    Stage stage;
    bool  omega;   // strips small platforms
    int   time;    // match time limit in seconds (-1 = off)
    bool items;
};

class StageSelectionScreen : public Screen {
public:
    StageSelectionScreen(const Stage& defaultStage, const std::vector<Stage>& stages);

    void handle(const SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

    bool isFinished()   const { return finished; }
    StageSelectionResult getResult() const { return result; }
    void resetFinished();

private:
    std::vector<Stage> stages;
    int   selectedIdx  = 0;
    bool  omega        = false;
    int   timeLimit    = -1;  // -1 = off, else seconds
    bool  items        = true;

    bool  finished = false;
    StageSelectionResult result;

    // layout constants
    static constexpr int PREVIEW_W = 700;
    static constexpr int PREVIEW_H = 394;  // 16:9 of PREVIEW_W
    static constexpr int PREVIEW_X = (1920 - PREVIEW_W) / 2;
    static constexpr int PREVIEW_Y = 220;

    void navigate(int dir);  // -1 left, +1 right
    void confirm();
    void renderPreview(SDL_Renderer* r, const Stage& stage) const;
};
