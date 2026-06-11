#pragma once
#include "game/billiards/GameTypes.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"

class BallManager;

class CueController {
public:
    void Init(BallManager* ballMgr);
    void Update(float dt, Scene& scene, Renderer& renderer, GameState& state);

    Vec3  GetAimDir()  const { return m_aimDir; }
    float GetPower()   const { return m_power; }
    Vec2  GetImpact()  const { return m_impactOffset; }
    void  SetImpact(const Vec2& offset) { m_impactOffset = offset; }

    static constexpr float AIM_SPEED  = 80.0f;
    static constexpr float MAX_SPEED  = 30.0f;
    static constexpr float MAX_CHARGE = 2.0f;
    static constexpr float SPIN_SCALE = 30.0f;

private:
    void DrawAimLine(Renderer& renderer, const Vec3& cueBallPos);

    BallManager* m_ballMgr     = nullptr;
    float        m_aimAngle    = 200.0f;
    Vec3         m_aimDir      = {-1, 0, 0};
    float        m_chargeTime  = 0.0f;
    float        m_power       = 0.0f;
    bool         m_wasDown     = false;
    Vec2         m_impactOffset = {0.0f, 0.0f};
};
