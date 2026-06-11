#pragma once

#include "app/IScene.h"
#include "ecs/World.hpp"
#include "game/billiards/BallManager.h"
#include "game/billiards/BilliardPhysicsSystem.h"
#include "game/billiards/BilliardsUI.h"
#include "game/billiards/CueController.h"
#include "game/billiards/GameTypes.h"
#include "game/billiards/PocketSystem.h"
#include "game/billiards/SpinDialUI.h"
#include "game/billiards/TableSetup.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "ui/UIRenderer.h"
#include <cassert>
#include <memory>

class BilliardsScene : public IScene {
public:
    BilliardsScene();
    ~BilliardsScene() override;

    void OnEnter()              override;
    void OnExit()               override;
    void OnUpdate(float dt)     override;
    void OnFixedUpdate()        override;
    void OnRender()             override;

    Renderer&   GetRenderer()    { assert(m_renderer); return *m_renderer; }
    UIRenderer& GetUIRenderer()  { return m_uiRenderer; }
    Scene&      GetScene()       { return m_scene; }
    GameState   GetState() const { return m_state; }
    void        SetState(GameState s) { m_state = s; }

private:
    static constexpr float FIXED_DT = 1.0f / 60.0f;

    std::unique_ptr<Renderer> m_renderer;
    UIRenderer m_uiRenderer;
    Scene      m_scene;
    World      m_world{m_scene.GetRegistry()};
    TableSetup m_tableSetup;
    BallManager m_ballManager;
    BilliardPhysicsSystem m_billiardPhysics;
    CueController m_cueController;
    SpinDialUI m_spinDial;
    BilliardsUI m_billiardsUI;
    PocketSystem m_pocketSystem;
    GameState  m_state    = GameState::Aiming;
    Entity     m_camEntity{};
    float      m_camAngle = 0.0f;  // radians, azimuth around Y
};
