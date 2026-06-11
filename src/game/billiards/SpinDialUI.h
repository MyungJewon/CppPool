#pragma once

#include "math/Vec2.h"

class UIRenderer;

class SpinDialUI {
public:
    void Update(int screenWidth, int screenHeight);
    void Render(UIRenderer& renderer, int screenWidth, int screenHeight, bool aiming) const;

    Vec2 GetOffset() const { return m_offset; }

    static constexpr float DIAL_CX = 90.0f;
    static constexpr float DIAL_CY_BOT = 90.0f;
    static constexpr float DIAL_R = 40.0f;

private:
    Vec2 m_offset = {0.0f, 0.0f};
};
