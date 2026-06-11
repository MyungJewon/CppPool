#pragma once

#include "ecs/Registry.hpp"

class BilliardPhysicsSystem {
public:
    static constexpr float BALL_RADIUS   = 0.25f;
    static constexpr float FRICTION_COEF = 0.2f;
    static constexpr float ROLL_RESIST   = 0.015f;
    static constexpr float GRAVITY       = 9.8f;
    static constexpr float SLIDE_THRESH  = 0.02f;

    void Update(Registry& reg, float dt);
};
