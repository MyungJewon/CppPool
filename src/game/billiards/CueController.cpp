#include "game/billiards/CueController.h"
#include "game/billiards/BallManager.h"
#include "input/InputManager.h"
#include "physics/RigidBody.h"
#include "renderer/Renderer.h"
#include "scene/Transform.h"
#include <algorithm>
#include <cmath>

static constexpr float DEG2RAD = 3.14159265f / 180.0f;

void CueController::Init(BallManager* ballMgr) {
    m_ballMgr = ballMgr;
}

void CueController::Update(float dt, Scene& scene, Renderer& renderer, GameState& state) {
    if (state != GameState::Aiming) return;

    auto& input = InputManager::Get();
    auto& reg   = scene.GetRegistry();

    Entity cueBall = m_ballMgr->GetCueBall();
    if (cueBall == NULL_ENTITY) return;

    // A/D rotation - use actual KeyCode enum values from InputManager.h
    if (input.IsKeyDown(KeyCode::A)) m_aimAngle -= AIM_SPEED * dt;
    if (input.IsKeyDown(KeyCode::D)) m_aimAngle += AIM_SPEED * dt;

    m_aimDir = Vec3{
        std::cos(m_aimAngle * DEG2RAD),
        0.0f,
        std::sin(m_aimAngle * DEG2RAD)
    };

    Vec3 cueBallPos = reg.get<Transform>(cueBall).localPos;

    // Power charge - use actual MouseButton enum value from InputManager.h
    bool isDown = input.IsMouseDown(MouseButton::Left);
    if (isDown) {
        m_chargeTime = std::min(m_chargeTime + dt, MAX_CHARGE);
    }
    m_power = m_chargeTime / MAX_CHARGE;

    if (m_wasDown && !isDown && m_chargeTime > 0.01f) {
        auto& rb = reg.get<RigidBody>(cueBall);
        rb.velocity       = m_aimDir * (m_power * MAX_SPEED);
        rb.angularVelocity.z += m_impactOffset.y * SPIN_SCALE;
        rb.angularVelocity.x -= m_impactOffset.x * SPIN_SCALE;
        m_chargeTime = 0.0f;
        m_power      = 0.0f;
        state        = GameState::Shooting;
    }
    m_wasDown = isDown;

    DrawAimLine(renderer, cueBallPos);
}

void CueController::DrawAimLine(Renderer& renderer, const Vec3& cueBallPos) {
    Vec3 end = cueBallPos + m_aimDir * 6.0f;
    renderer.DrawLine(cueBallPos, end, {1.0f, 1.0f, 0.0f});
}
