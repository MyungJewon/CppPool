#pragma once

#include "ecs/Entity.hpp"
#include "math/Vec3.h"
#include "resource/ObjLoader.h"
#include "resource/Texture.h"

class Scene;

class TableSetup {
public:
    void Init(Scene& scene);

    static constexpr float TABLE_W   = 18.0f;
    static constexpr float TABLE_D   = 9.0f;
    static constexpr float CUSHION_H = 0.5f;
    static constexpr float CUSHION_T = 0.35f;

private:
    void CreateSurface(Scene& scene);
    void CreateCushions(Scene& scene);
    void CreatePockets(Scene& scene);

    Mesh    m_surfaceMesh;
    Mesh    m_cushionMesh[4];
    Mesh    m_pocketMesh;
    Texture m_feltTex;
    Texture m_cushionTex;
    Texture m_pocketTex;
};
