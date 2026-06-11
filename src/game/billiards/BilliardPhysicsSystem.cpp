#include "game/billiards/BilliardPhysicsSystem.h"

#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
#include <algorithm>
#include <cmath>

void BilliardPhysicsSystem::Update(Registry& reg, float dt) {
    for (auto [rb, col, tf] : reg.view<RigidBody, Collider, Transform>()) {
        if (rb.isKinematic || col.shape != ColliderShape::Sphere) {
            continue;
        }

        // Keep ball on table surface; also acts as floor clamp if CollisionSystem misses
        if (tf.localPos.y < BALL_RADIUS) {
            tf.localPos.y = BALL_RADIUS;
            if (rb.velocity.y < 0.0f) rb.velocity.y = 0.0f;
        }

        if (std::fabs(tf.localPos.y - BALL_RADIUS) >= 0.08f) {
            continue;
        }

        const float r = col.radius > 0.0f ? col.radius : BALL_RADIUS;
        const float contactVx = rb.velocity.x - (-rb.angularVelocity.z * r);
        const float contactVz = rb.velocity.z - ( rb.angularVelocity.x * r);
        const float slipLen = std::sqrt(contactVx * contactVx + contactVz * contactVz);

        if (slipLen > SLIDE_THRESH) {
            const float slipX = contactVx / slipLen;
            const float slipZ = contactVz / slipLen;
            const float fAcc = FRICTION_COEF * GRAVITY;
            // α = F*r/I, I = 2/5*m*r², m=1 → α = 5/2 * F/r
            float angAcc = 2.5f * fAcc / r;

            rb.velocity.x -= slipX * fAcc * dt;
            rb.velocity.z -= slipZ * fAcc * dt;
            rb.angularVelocity.x += slipZ * angAcc * dt;
            rb.angularVelocity.z -= slipX * angAcc * dt;
        } else {
            float decay = 1.0f - ROLL_RESIST;  // constant per fixed step; ROLL_RESIST tuned for 60Hz

            rb.velocity.x *= decay;
            rb.velocity.z *= decay;
            rb.angularVelocity.x *= decay;
            rb.angularVelocity.z *= decay;
        }
    }
}
