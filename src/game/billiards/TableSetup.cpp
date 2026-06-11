#include "game/billiards/TableSetup.h"

#include "core/Color.h"
#include "physics/Collider.h"
#include "renderer/MeshRenderer.h"
#include "resource/MeshGenerator.h"
#include "scene/Scene.h"
#include "scene/Transform.h"
#include <vector>

static constexpr float POCKET_R  = 0.50f;
static constexpr float POCKET_VIS = POCKET_R * 2.2f;

void TableSetup::Init(Scene& scene) {
    CreateSurface(scene);
    CreateCushions(scene);
    CreatePockets(scene);
}

void TableSetup::CreateSurface(Scene& scene) {
    m_surfaceMesh = MeshGenerator::CreateBox(TABLE_W, 0.1f, TABLE_D);

    std::vector<Color> pixels(8 * 8, Color(30, 100, 30, 255));
    m_feltTex = Texture::FromPixels(8, 8, pixels);

    Entity surfaceEntity = scene.CreateEntity();

    Transform transform;
    transform.localPos = {0.0f, -0.05f, 0.0f};
    scene.GetRegistry().add<Transform>(surfaceEntity, transform);

    MeshRenderer renderer;
    renderer.mesh = &m_surfaceMesh;
    renderer.material.albedo = &m_feltTex;
    renderer.material.normalMap = nullptr;
    renderer.material.tint = {1.0f, 1.0f, 1.0f};
    renderer.material.shininess = 16.0f;
    renderer.meshName = "billiards_table_surface";
    scene.GetRegistry().add<MeshRenderer>(surfaceEntity, renderer);

    Collider collider;
    collider.shape = ColliderShape::AABB;
    collider.center = {0.0f, 0.0f, 0.0f};
    collider.halfExtents = {TABLE_W * 0.5f, 0.05f, TABLE_D * 0.5f};
    collider.restitution = 0.2f;
    collider.friction = 0.6f;
    scene.GetRegistry().add<Collider>(surfaceEntity, collider);
}

void TableSetup::CreateCushions(Scene& scene) {
    std::vector<Color> pixels(4 * 4, Color(90, 50, 15, 255));
    m_cushionTex = Texture::FromPixels(4, 4, pixels);

    const Vec3 positions[4] = {
        {0.0f, CUSHION_H * 0.5f, -(TABLE_D * 0.5f + CUSHION_T * 0.5f)},
        {0.0f, CUSHION_H * 0.5f,  (TABLE_D * 0.5f + CUSHION_T * 0.5f)},
        {-(TABLE_W * 0.5f + CUSHION_T * 0.5f), CUSHION_H * 0.5f, 0.0f},
        { (TABLE_W * 0.5f + CUSHION_T * 0.5f), CUSHION_H * 0.5f, 0.0f}
    };

    const Vec3 sizes[4] = {
        {TABLE_W + CUSHION_T * 2.0f, CUSHION_H, CUSHION_T},
        {TABLE_W + CUSHION_T * 2.0f, CUSHION_H, CUSHION_T},
        {CUSHION_T, CUSHION_H, TABLE_D},
        {CUSHION_T, CUSHION_H, TABLE_D}
    };

    const char* names[4] = {
        "billiards_cushion_back",
        "billiards_cushion_front",
        "billiards_cushion_left",
        "billiards_cushion_right"
    };

    for (int i = 0; i < 4; ++i) {
        m_cushionMesh[i] = MeshGenerator::CreateBox(sizes[i].x, sizes[i].y, sizes[i].z);

        Entity cushionEntity = scene.CreateEntity();

        Transform transform;
        transform.localPos = positions[i];
        scene.GetRegistry().add<Transform>(cushionEntity, transform);

        MeshRenderer renderer;
        renderer.mesh = &m_cushionMesh[i];
        renderer.material.albedo = &m_cushionTex;
        renderer.material.normalMap = nullptr;
        renderer.material.tint = {1.0f, 1.0f, 1.0f};
        renderer.material.shininess = 24.0f;
        renderer.meshName = names[i];
        scene.GetRegistry().add<MeshRenderer>(cushionEntity, renderer);

        Collider collider;
        collider.shape = ColliderShape::AABB;
        collider.center = {0.0f, 0.0f, 0.0f};
        collider.halfExtents = {sizes[i].x * 0.5f, sizes[i].y * 0.5f, sizes[i].z * 0.5f};
        collider.restitution = 0.75f;
        collider.friction = 0.1f;
        scene.GetRegistry().add<Collider>(cushionEntity, collider);
    }
}

void TableSetup::CreatePockets(Scene& scene) {
    const Vec3 positions[6] = {
        {-(TABLE_W * 0.5f), 0.0f, -(TABLE_D * 0.5f)},
        { (TABLE_W * 0.5f), 0.0f, -(TABLE_D * 0.5f)},
        {-(TABLE_W * 0.5f), 0.0f,  (TABLE_D * 0.5f)},
        { (TABLE_W * 0.5f), 0.0f,  (TABLE_D * 0.5f)},
        {0.0f, 0.0f, -(TABLE_D * 0.5f)},
        {0.0f, 0.0f,  (TABLE_D * 0.5f)}
    };

    m_pocketMesh = MeshGenerator::CreateBox(POCKET_VIS, 0.02f, POCKET_VIS);
    std::vector<Color> pxPocket(4 * 4, Color(10, 10, 10, 255));
    m_pocketTex = Texture::FromPixels(4, 4, pxPocket);

    for (const Vec3& position : positions) {
        Entity pocketEntity = scene.CreateEntity();

        Transform transform;
        transform.localPos = {position.x, 0.01f, position.z}; // slightly above surface to prevent z-fighting
        scene.GetRegistry().add<Transform>(pocketEntity, transform);

        // Dark visual marker on the table surface
        MeshRenderer mr;
        mr.mesh = &m_pocketMesh;
        mr.material.albedo = &m_pocketTex;
        mr.material.tint = {0.05f, 0.05f, 0.05f};
        mr.material.shininess = 4.0f;
        mr.meshName = "pocket_visual";
        scene.GetRegistry().add<MeshRenderer>(pocketEntity, mr);

        Collider collider;
        collider.shape = ColliderShape::Sphere;
        collider.center = {0.0f, 0.0f, 0.0f};
        collider.radius = POCKET_R;
        collider.isTrigger = true;
        scene.GetRegistry().add<Collider>(pocketEntity, collider);
    }
}
