#include "game/billiards/BallManager.h"
#include "resource/MeshGenerator.h"
#include "renderer/MeshRenderer.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
#include "scene/Scene.h"
#include <cmath>
#include <algorithm>

static const Vec3 BALL_COLORS[16] = {
    {1.0f, 1.0f, 1.0f},
    {1.0f, 0.9f, 0.0f},
    {0.1f, 0.2f, 0.8f},
    {0.8f, 0.1f, 0.1f},
    {0.5f, 0.1f, 0.5f},
    {0.9f, 0.4f, 0.1f},
    {0.1f, 0.5f, 0.1f},
    {0.6f, 0.1f, 0.1f},
    {0.05f,0.05f,0.05f},
    {1.0f, 0.9f, 0.0f},
    {0.1f, 0.2f, 0.8f},
    {0.8f, 0.1f, 0.1f},
    {0.5f, 0.1f, 0.5f},
    {0.9f, 0.4f, 0.1f},
    {0.1f, 0.5f, 0.1f},
    {0.6f, 0.1f, 0.1f},
};

void BallManager::Init(Scene& scene) {
    m_ballMesh = MeshGenerator::CreateSphere(16, 16, BALL_R);

    m_cueBall = CreateBall(scene, {CUE_START_X, CUE_START_Y, 0.0f}, BALL_COLORS[0]);
    PlaceRack(scene);
}

Entity BallManager::CreateBall(Scene& scene, const Vec3& pos, const Vec3& tint) {
    Entity e = scene.CreateEntity();
    auto& reg = scene.GetRegistry();

    Transform t{};
    t.localPos = pos;
    reg.add<Transform>(e, t);

    MeshRenderer mr{};
    mr.mesh = &m_ballMesh;
    mr.material.tint = tint;
    reg.add<MeshRenderer>(e, mr);

    Collider col{};
    col.shape = ColliderShape::Sphere;
    col.radius = BALL_R;
    col.restitution = 0.9f;
    col.friction = 0.3f;
    col.isTrigger = false;
    reg.add<Collider>(e, col);

    RigidBody rb{};
    rb.mass = 1.0f;
    rb.drag = 0.4f;
    rb.angularDrag = 0.5f;
    rb.useGravity = true;
    rb.isKinematic = false;
    reg.add<RigidBody>(e, rb);

    return e;
}

void BallManager::PlaceRack(Scene& scene) {
    int order[15] = {1, 2,9, 3,8,10, 4,14,13,11, 5,6,7,15,12};
    float spacing = BALL_R * 2.0f + 0.005f;  // tiny gap to prevent initial interpenetration
    float rowDX   = spacing * sqrtf(3.0f) * 0.5f;
    float rowDZ   = spacing;

    int idx = 0;
    for (int row = 0; row < 5; ++row) {
        int count = row + 1;
        float startZ = -(count - 1) * rowDZ * 0.5f;
        for (int col = 0; col < count; ++col) {
            Vec3 pos = {RACK_TIP_X + row * rowDX, CUE_START_Y, startZ + col * rowDZ};
            int ballNum = order[idx++];
            Entity e = CreateBall(scene, pos, BALL_COLORS[ballNum]);
            m_numberBalls.push_back(e);
        }
    }
}

void BallManager::RemoveBall(Scene& scene, Entity e) {
    scene.DestroyEntity(e);
    if (e == m_cueBall) {
        m_cueBall = NULL_ENTITY;
    } else {
        auto it = std::find(m_numberBalls.begin(), m_numberBalls.end(), e);
        if (it != m_numberBalls.end())
            m_numberBalls.erase(it);
    }
}

void BallManager::Reset(Scene& scene) {
    if (m_cueBall != NULL_ENTITY) {
        scene.DestroyEntity(m_cueBall);
        m_cueBall = NULL_ENTITY;
    }
    for (Entity e : m_numberBalls)
        scene.DestroyEntity(e);
    m_numberBalls.clear();
    Init(scene);
}
