#pragma once

#include "game/billiards/GameTypes.h"
#include <string>

class UIRenderer;

class BilliardsUI {
public:
    void ShowFoul(float duration = 2.0f);
    void Update(float dt);
    void Render(UIRenderer& ui, int w, int h, GameState state,
                float power, int ballsRemaining);

private:
    float m_foulTimer = 0.0f;
};
