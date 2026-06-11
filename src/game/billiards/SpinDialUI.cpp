#include "game/billiards/SpinDialUI.h"
#include "input/InputManager.h"
#include "math/Vec3.h"
#include "ui/UIRenderer.h"
#include <cmath>

void SpinDialUI::Update(int screenWidth, int screenHeight) {
    (void)screenWidth;

    auto& input = InputManager::Get();
    if (!input.IsMouseDown(MouseButton::Right)) return;

    const float cx = DIAL_CX;
    const float cy = static_cast<float>(screenHeight) - DIAL_CY_BOT;
    Vec2 offset{
        (static_cast<float>(input.MouseX()) - cx) / DIAL_R,
        (cy - static_cast<float>(input.MouseY())) / DIAL_R
    };

    const float len = offset.length();
    if (len > 1.0f) {
        offset = offset / len;
    }

    m_offset = offset;
}

void SpinDialUI::Render(UIRenderer& renderer, int screenWidth, int screenHeight, bool aiming) const {
    (void)screenWidth;

    if (!aiming) return;

    const float cx = DIAL_CX;
    const float cy = static_cast<float>(screenHeight) - DIAL_CY_BOT;
    const float left = cx - DIAL_R;
    const float top = cy - DIAL_R;
    const float size = DIAL_R * 2.0f;

    renderer.DrawRect(left, top, size, size, {0.02f, 0.025f, 0.03f}, 0.86f);
    renderer.DrawRect(left, cy - 1.0f, size, 2.0f, {0.75f, 0.78f, 0.82f}, 0.65f);
    renderer.DrawRect(cx - 1.0f, top, 2.0f, size, {0.75f, 0.78f, 0.82f}, 0.65f);

    const float dotSize = 8.0f;
    const float dotX = cx + m_offset.x * DIAL_R - dotSize * 0.5f;
    const float dotY = cy - m_offset.y * DIAL_R - dotSize * 0.5f;
    renderer.DrawRect(dotX, dotY, dotSize, dotSize, {1.0f, 0.88f, 0.12f}, 1.0f);

    renderer.DrawText(left + 20.0f, top - 22.0f, "SPIN", 1, {1.0f, 1.0f, 1.0f}, 0.9f);
}
