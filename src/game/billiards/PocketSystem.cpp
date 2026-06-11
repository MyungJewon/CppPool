#include "game/billiards/PocketSystem.h"

#include "event/EventBus.h"
#include "event/Events.h"
#include "game/billiards/BallManager.h"
#include "game/billiards/BilliardsUI.h"
#include "physics/RigidBody.h"
#include "scene/Scene.h"
#include "scene/Transform.h"

PocketSystem::~PocketSystem() {
    EventBus::Clear();
}

void PocketSystem::Init(Scene& scene, BallManager* ballMgr) {
    (void)scene;
    m_ballMgr = ballMgr;

    EventBus::Subscribe<TriggerEnterEvent>([this](const TriggerEnterEvent& e) {
        Entity entering = e.other;
        m_toRemove.push(entering);
        if (entering == m_ballMgr->GetCueBall()) {
            m_cueBallPocketed = true;
        }
    });
}

void PocketSystem::Update(Scene& scene, GameState& state, BilliardsUI* ui) {
    while (!m_toRemove.empty()) {
        Entity e = m_toRemove.front();
        m_toRemove.pop();

        if (!scene.GetRegistry().has<Transform>(e)) {
            continue;
        }

        if (e == m_ballMgr->GetCueBall()) {
            auto& tf = scene.GetRegistry().get<Transform>(e);
            tf.localPos = Vec3{BallManager::CUE_START_X, BallManager::CUE_START_Y, 0.0f};
            auto& rb = scene.GetRegistry().get<RigidBody>(e);
            rb.velocity        = {0, 0, 0};
            rb.angularVelocity = {0, 0, 0};
            m_cueBallPocketed  = false;
            state = GameState::Aiming;
            if (ui) ui->ShowFoul();
        } else if (scene.GetRegistry().has<RigidBody>(e)) {
            m_ballMgr->RemoveBall(scene, e);
        }
    }
}
