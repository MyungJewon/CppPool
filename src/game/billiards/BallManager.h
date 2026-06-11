#pragma once
#include "ecs/Entity.hpp"
#include "math/Vec3.h"
#include "resource/ObjLoader.h"
#include <vector>

class Scene;

class BallManager {
public:
    void Init(Scene& scene);
    void Reset(Scene& scene);

    Entity GetCueBall() const { return m_cueBall; }
    const std::vector<Entity>& GetNumberBalls() const { return m_numberBalls; }
    void RemoveBall(Scene& scene, Entity e);

    static constexpr float BALL_R    = 0.25f;
    static constexpr float CUE_START_X = -6.0f;
    static constexpr float CUE_START_Y =  0.25f;
    static constexpr float RACK_TIP_X  =  4.0f;

private:
    Entity CreateBall(Scene& scene, const Vec3& pos, const Vec3& tint);
    void   PlaceRack(Scene& scene);

    Entity m_cueBall = NULL_ENTITY;
    std::vector<Entity> m_numberBalls;

    Mesh    m_ballMesh;
};
