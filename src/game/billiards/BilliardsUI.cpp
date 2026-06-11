#include "game/billiards/BilliardsUI.h"

#include "ui/UIRenderer.h"

void BilliardsUI::ShowFoul(float duration) {
    m_foulTimer = duration;
}

void BilliardsUI::Update(float dt) {
    if (m_foulTimer > 0.0f) {
        m_foulTimer -= dt;
        if (m_foulTimer < 0.0f) m_foulTimer = 0.0f;
    }
}

void BilliardsUI::Render(UIRenderer& ui, int w, int h, GameState state,
                         float power, int ballsRemaining) {
    const char* stateStr = "AIMING";
    if (state == GameState::Shooting)  stateStr = "SHOOTING";
    if (state == GameState::Resetting) stateStr = "RESETTING";
    ui.DrawText(16.0f, 16.0f, stateStr, 18, {1.0f, 1.0f, 1.0f}, 0.9f);

    ui.DrawText(16.0f, 42.0f, "Balls: " + std::to_string(ballsRemaining),
                14, {0.8f, 0.8f, 0.8f}, 0.8f);

    float barW = 120.0f, barH = 12.0f;
    float barX = static_cast<float>(w) - barW - 16.0f;
    float barY = static_cast<float>(h) - 40.0f;
    ui.DrawText(barX, barY - 18.0f, "POWER", 12, {0.7f, 0.7f, 0.7f}, 0.8f);
    ui.DrawRect(barX, barY, barW, barH, {0.2f, 0.2f, 0.2f}, 0.7f);
    if (power > 0.0f)
        ui.DrawRect(barX, barY, barW * power, barH, {1.0f, 0.5f, 0.0f}, 0.9f);

    if (m_foulTimer > 0.0f) {
        float alpha = m_foulTimer > 1.0f ? 1.0f : m_foulTimer;
        ui.DrawText(static_cast<float>(w) * 0.5f - 28.0f,
                    static_cast<float>(h) * 0.5f,
                    "FOUL!", 24, {1.0f, 0.3f, 0.3f}, alpha);
    }

    ui.DrawText(static_cast<float>(w) - 80.0f, 16.0f,
                "[R] Reset", 12, {0.5f, 0.5f, 0.5f}, 0.7f);

    if (state == GameState::Aiming) {
        ui.DrawText(16.0f, static_cast<float>(h) - 60.0f,
                    "A/D: Aim", 12, {0.6f, 0.6f, 0.6f}, 0.8f);
        ui.DrawText(16.0f, static_cast<float>(h) - 44.0f,
                    "Hold LMB: Charge  Release: Shoot", 12, {0.6f, 0.6f, 0.6f}, 0.8f);
        ui.DrawText(16.0f, static_cast<float>(h) - 28.0f,
                    "RMB Drag: Spin  Q/E: Camera", 12, {0.6f, 0.6f, 0.6f}, 0.8f);
    }
}
