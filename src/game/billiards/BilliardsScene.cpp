#include "game/billiards/BilliardsScene.h"
#include "app/Application.h"
#include "audio/AudioManager.h"
#include "core/Path.h"
#include "input/InputManager.h"
#include "math/MathUtils.h"
#include <cmath>
#include "renderer/Renderer.h"
#include "physics/RigidBody.h"
#include "scene/Camera.h"
#include "scene/Transform.h"
#include "scene/Light.h"
#include "systems/CollisionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderSystem.h"
#include "systems/UISystem.h"

BilliardsScene::BilliardsScene() = default;
BilliardsScene::~BilliardsScene() = default;

void BilliardsScene::OnEnter() {
    auto& win = m_app->GetWindow();
    m_renderer = std::make_unique<Renderer>(win.PixelWidth(), win.PixelHeight());
    m_uiRenderer.Init(win.PixelWidth(), win.PixelHeight());
    AudioManager::Get().Init();

    // Quarter-view fixed camera
    Entity camEntity = m_scene.CreateEntity();
    Camera cam;
    cam.eye    = {0.0f, 14.0f, 10.0f};
    cam.target = {0.0f, 0.0f, 0.0f};
    cam.up     = {0.0f, 1.0f, 0.0f};
    cam.fovY   = DegToRad(60.0f);
    cam.aspect = win.Width() / static_cast<float>(win.Height());
    cam.zNear  = 0.1f;
    cam.zFar   = 100.0f;
    m_scene.GetRegistry().add<Camera>(camEntity, cam);
    m_scene.SetActiveCamera(camEntity);
    m_camEntity = camEntity;

    // Light
    Entity lightEntity = m_scene.CreateEntity();
    Light light;
    light.position = {0.0f, 16.0f, 4.0f};
    light.color    = {1.0f, 1.0f, 1.0f};
    light.ambient  = 0.45f;
    light.diffuse  = 0.75f;
    light.specular = 0.4f;
    light.shininess= 32.0f;
    m_scene.GetRegistry().add<Light>(lightEntity, light);
    m_scene.SetActiveLight(lightEntity);

    // Register systems
    m_world.add_system<RenderSystem>(*m_renderer, m_scene, m_app->GetWindow(), nullptr); // nullptr = no skybox
    m_world.add_system<UISystem>(m_uiRenderer, m_app->GetWindow());
    m_world.add_fixed_system<PhysicsSystem>();
    m_world.add_fixed_system<CollisionSystem>();

    m_tableSetup.Init(m_scene);
    m_ballManager.Init(m_scene);
    m_pocketSystem.Init(m_scene, &m_ballManager);
    m_cueController.Init(&m_ballManager);
}

void BilliardsScene::OnExit() {}

void BilliardsScene::OnUpdate(float dt) {
    // Q/E orbit camera around table center
    {
        auto& input = InputManager::Get();
        constexpr float CAM_SPEED = 1.5f;  // rad/s
        if (input.IsKeyDown(KeyCode::Q)) m_camAngle += CAM_SPEED * dt;
        if (input.IsKeyDown(KeyCode::E)) m_camAngle -= CAM_SPEED * dt;

        constexpr float CAM_R = 10.0f, CAM_H = 14.0f;
        auto& cam = m_scene.GetRegistry().get<Camera>(m_camEntity);

        Vec3 focus = {0.0f, 0.0f, 0.0f};
        {
            Entity cue = m_ballManager.GetCueBall();
            if (m_scene.GetRegistry().has<Transform>(cue))
                focus = m_scene.GetRegistry().get<Transform>(cue).localPos;
        }

        cam.target = focus;
        cam.eye = { focus.x + std::sin(m_camAngle) * CAM_R,
                    focus.y + CAM_H,
                    focus.z + std::cos(m_camAngle) * CAM_R };
    }

    m_world.update(dt);
    auto& win = m_app->GetWindow();
    if (m_state == GameState::Aiming) {
        m_spinDial.Update(win.PixelWidth(), win.PixelHeight());
        m_cueController.SetImpact(m_spinDial.GetOffset());
    }
    m_cueController.Update(dt, m_scene, GetRenderer(), m_state);
    m_pocketSystem.Update(m_scene, m_state, &m_billiardsUI);

    m_billiardsUI.Update(dt);
    if (InputManager::Get().JustPressed(KeyCode::R)) {
        m_ballManager.Reset(m_scene);
        m_state = GameState::Aiming;
    }

    if (m_state == GameState::Shooting) {
        bool allStopped = true;
        for (auto& rb : m_scene.GetRegistry().pool<RigidBody>()) {
            float vLen = rb.velocity.x*rb.velocity.x +
                         rb.velocity.y*rb.velocity.y +
                         rb.velocity.z*rb.velocity.z;
            float wLen = rb.angularVelocity.x*rb.angularVelocity.x +
                         rb.angularVelocity.z*rb.angularVelocity.z;
            if (vLen > 0.04f || wLen > 0.04f) { allStopped = false; break; }
        }
        if (allStopped) m_state = GameState::Aiming;
    }
}

void BilliardsScene::OnFixedUpdate() {
    m_world.fixed_update(FIXED_DT);
    m_billiardPhysics.Update(m_scene.GetRegistry(), FIXED_DT);
}

void BilliardsScene::OnRender() {
    auto& win = m_app->GetWindow();
    m_uiRenderer.BeginFrame(win.PixelWidth(), win.PixelHeight());
    m_spinDial.Render(m_uiRenderer, win.PixelWidth(), win.PixelHeight(),
                      m_state == GameState::Aiming);
    m_billiardsUI.Render(m_uiRenderer, win.PixelWidth(), win.PixelHeight(),
                         m_state, m_cueController.GetPower(),
                         static_cast<int>(m_ballManager.GetNumberBalls().size()));
    m_uiRenderer.EndFrame();
    m_app->GetWindow().SwapBuffers();
}
