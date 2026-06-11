# CppPool Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Cpp_Engine v0.5.0 기반 솔로 자유 연습 포켓볼 게임 — 물리 기반 스핀 포함

**Architecture:** Cpp_Engine 소스를 CppPool 레포에 일체형으로 복사. `src/game/billiards/`에 게임 코드 추가. `BilliardsScene(IScene)`이 게임 전체를 소유하고 `BilliardsApp(Application)`이 씬을 로드. 엔진 원본 파일은 수정하지 않음.

**Tech Stack:** C++17, Cpp_Engine v0.5.0 (ECS, PhysicsSystem, CollisionSystem, GLRenderer, UIRenderer), CMake

---

## 파일 맵

| 파일 | 역할 |
|---|---|
| `src/main.cpp` | BilliardsApp으로 교체 |
| `CMakeLists.txt` | 게임 소스 추가 |
| `src/game/billiards/GameTypes.h` | GameState enum 정의 (순환 include 방지) |
| `src/game/billiards/BilliardsApp.h/.cpp` | Application 진입점, BilliardsScene 로드 |
| `src/game/billiards/BilliardsScene.h/.cpp` | IScene 구현, Renderer/World/Scene 소유, 상태 관리 |
| `src/game/billiards/TableSetup.h/.cpp` | 테이블·쿠션·포켓 Entity 생성 |
| `src/game/billiards/BallManager.h/.cpp` | 공 16개 생성·관리·리셋 |
| `src/game/billiards/CueController.h/.cpp` | A/D 조준각, 좌클릭 파워, 발사 |
| `src/game/billiards/BilliardPhysicsSystem.h/.cpp` | 롤링 마찰 + 스핀→선속도 변환 |
| `src/game/billiards/PocketSystem.h/.cpp` | TriggerEnterEvent, 공 제거, 큐볼 복귀 |
| `src/game/billiards/SpinDialUI.h/.cpp` | 임팩트 포인트 HUD 다이얼 |
| `src/game/billiards/BilliardsUI.h/.cpp` | 미니멀 오버레이 HUD |

---

## Task 0: GameTypes.h — 공유 타입 정의

**Files:**
- Create: `src/game/billiards/GameTypes.h`

- [ ] **Step 1: GameTypes.h 생성**

`GameState`와 공용 상수를 한 곳에 모아 순환 include를 방지합니다.

```cpp
#pragma once

enum class GameState { Aiming, Shooting, Resetting };
```

---

## Task 1: 프로젝트 셋업

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`
- Create: `src/game/billiards/BilliardsApp.h`
- Create: `src/game/billiards/BilliardsApp.cpp`

- [ ] **Step 1: CMakeLists.txt에 게임 소스 추가**

`add_executable(Cpp_Engine ${ENGINE_SOURCES})` 윗줄에 아래 블록 추가:

```cmake
list(APPEND ENGINE_SOURCES
  src/game/billiards/BilliardsApp.cpp
  src/game/billiards/BilliardsScene.cpp
  src/game/billiards/TableSetup.cpp
  src/game/billiards/BallManager.cpp
  src/game/billiards/CueController.cpp
  src/game/billiards/BilliardPhysicsSystem.cpp
  src/game/billiards/PocketSystem.cpp
  src/game/billiards/SpinDialUI.cpp
  src/game/billiards/BilliardsUI.cpp
)
```

- [ ] **Step 2: BilliardsApp.h 생성**

```cpp
#pragma once
#include "app/Application.h"

class BilliardsApp : public Application {
public:
    BilliardsApp(int width, int height, const char* title);
protected:
    void OnInit() override;
};
```

- [ ] **Step 3: BilliardsApp.cpp 생성 (BilliardsScene은 다음 Task에서 구현)**

```cpp
#include "game/billiards/BilliardsApp.h"
#include "game/billiards/BilliardsScene.h"
#include <memory>

BilliardsApp::BilliardsApp(int width, int height, const char* title)
    : Application(width, height, title) {}

void BilliardsApp::OnInit() {
    LoadScene(std::make_unique<BilliardsScene>());
}
```

- [ ] **Step 4: main.cpp 교체**

```cpp
#include "game/billiards/BilliardsApp.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int, char**) {
#endif
    BilliardsApp app(1280, 720, "CppPool");
    app.Run();
    return 0;
}
```

---

## Task 2: BilliardsScene 뼈대 + 카메라

**Files:**
- Create: `src/game/billiards/BilliardsScene.h`
- Create: `src/game/billiards/BilliardsScene.cpp`

- [ ] **Step 1: BilliardsScene.h 생성**

```cpp
#pragma once
#include "app/IScene.h"
#include "audio/AudioClip.h"
#include "core/Path.h"
#include "ecs/World.hpp"
#include "renderer/Renderer.h"
#include "renderer/Skybox.h"
#include "scene/Scene.h"
#include "systems/RenderSystem.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/UISystem.h"
#include "ui/UIRenderer.h"
#include "game/billiards/GameTypes.h"

class BilliardsScene : public IScene {
public:
    void OnEnter()              override;
    void OnExit()               override;
    void OnUpdate(float dt)     override;
    void OnFixedUpdate()        override;
    void OnRender()             override;

    Renderer&   GetRenderer()   { return m_renderer; }
    UIRenderer& GetUIRenderer() { return m_uiRenderer; }
    Scene&      GetScene()      { return m_scene; }
    GameState   GetState() const { return m_state; }
    void        SetState(GameState s) { m_state = s; }

private:
    Renderer   m_renderer{1280, 720};
    UIRenderer m_uiRenderer;
    Skybox     m_skybox;
    Scene      m_scene;
    World      m_world{m_scene.GetRegistry()};
    GameState  m_state = GameState::Aiming;
};
```

- [ ] **Step 2: BilliardsScene.cpp 생성 — OnEnter (카메라 + 조명만)**

```cpp
#include "game/billiards/BilliardsScene.h"
#include "audio/AudioManager.h"
#include "scene/Camera.h"
#include "scene/Light.h"

void BilliardsScene::OnEnter() {
    auto& win = m_app->GetWindow();
    m_renderer = Renderer(win.PixelWidth(), win.PixelHeight());
    m_uiRenderer.Init(win.PixelWidth(), win.PixelHeight());
    m_skybox.Load(Path::GetExecutableDir() + "/assets/skybox/Epic_BlueSunset_EquiRect_flat.png");
    AudioManager::Get().Init();

    // 카메라 — 쿼터뷰 고정
    Entity camEntity = m_scene.CreateEntity();
    Camera cam;
    cam.eye    = {0.0f, 7.0f, 5.0f};
    cam.target = {0.0f, 0.0f, 0.0f};
    cam.up     = {0.0f, 1.0f, 0.0f};
    cam.fov    = 60.0f;
    cam.aspect = win.Width() / static_cast<float>(win.Height());
    cam.near_  = 0.1f;
    cam.far_   = 100.0f;
    m_scene.GetRegistry().add<Camera>(camEntity, cam);
    m_scene.SetActiveCamera(camEntity);

    // 조명
    Entity lightEntity = m_scene.CreateEntity();
    Light light;
    light.position = {0.0f, 8.0f, 2.0f};
    light.color    = {1.0f, 1.0f, 1.0f};
    light.ambient  = 0.25f;
    light.diffuse  = 0.75f;
    light.specular = 0.4f;
    light.shininess= 32.0f;
    m_scene.GetRegistry().add<Light>(lightEntity, light);
    m_scene.SetActiveLight(lightEntity);

    // 시스템 등록 (InputSystem은 Tab 카메라 전환 없이)
    m_world.add_system<RenderSystem>(m_renderer, m_scene, m_app->GetWindow(), nullptr);
    m_world.add_system<UISystem>(m_uiRenderer, m_app->GetWindow());
    m_world.add_fixed_system<PhysicsSystem>();
    m_world.add_fixed_system<CollisionSystem>();
}

void BilliardsScene::OnExit() {}

void BilliardsScene::OnUpdate(float dt) {
    m_world.update(dt);
}

void BilliardsScene::OnFixedUpdate() {
    m_world.fixed_update(1.0f / 60.0f);
}

void BilliardsScene::OnRender() {
    m_app->GetWindow().SwapBuffers();
}
```

- [ ] **Step 3: 빌드 & 실행 확인**

```bash
cd /Users/deepfine/C++Project/CppPool/build && make -j4
./bin/Cpp_Engine
```

기대 결과: 창이 열리고 스카이박스 배경이 보임. 오류 없이 실행.

---

## Task 3: TableSetup — 테이블·쿠션·포켓

**Files:**
- Create: `src/game/billiards/TableSetup.h`
- Create: `src/game/billiards/TableSetup.cpp`
- Modify: `src/game/billiards/BilliardsScene.h` (TableSetup 멤버 추가)
- Modify: `src/game/billiards/BilliardsScene.cpp` (OnEnter에서 TableSetup 호출)

- [ ] **Step 1: TableSetup.h 생성**

```cpp
#pragma once
#include "ecs/Entity.hpp"
#include "scene/Scene.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"
#include <array>

class TableSetup {
public:
    void Init(Scene& scene);

    static constexpr float TABLE_W  = 9.0f;
    static constexpr float TABLE_D  = 4.5f;
    static constexpr float CUSHION_H = 0.3f;
    static constexpr float CUSHION_T = 0.2f;
    static constexpr float BALL_R   = 0.25f;

private:
    void CreateSurface(Scene& scene);
    void CreateCushions(Scene& scene);
    void CreatePockets(Scene& scene);

    Mesh m_surfaceMesh;
    Mesh m_cushionMesh[4];
    Mesh m_pocketMesh;
    Texture m_feltTex;
    Texture m_cushionTex;
};
```

- [ ] **Step 2: TableSetup.cpp 생성**

```cpp
#include "game/billiards/TableSetup.h"
#include "physics/Collider.h"
#include "renderer/MeshRenderer.h"
#include "resource/MeshGenerator.h"
#include "scene/Transform.h"
#include "core/Color.h"

void TableSetup::Init(Scene& scene) {
    CreateSurface(scene);
    CreateCushions(scene);
    CreatePockets(scene);
}

void TableSetup::CreateSurface(Scene& scene) {
    m_surfaceMesh = MeshGenerator::CreateBox(TABLE_W, 0.1f, TABLE_D);

    // 초록 펠트 텍스처 (8x8 단색)
    std::vector<Color> pixels(8 * 8, Color(30, 100, 30, 255));
    m_feltTex = Texture::FromPixels(8, 8, pixels);

    Entity e = scene.CreateEntity();
    Transform tf;
    tf.localPos = {0.0f, -0.05f, 0.0f};
    scene.GetRegistry().add<Transform>(e, tf);

    MeshRenderer mr;
    mr.mesh = &m_surfaceMesh;
    mr.material.albedo = &m_feltTex;
    scene.GetRegistry().add<MeshRenderer>(e, mr);

    Collider col;
    col.shape       = ColliderShape::AABB;
    col.halfExtents = {TABLE_W * 0.5f, 0.05f, TABLE_D * 0.5f};
    col.restitution = 0.2f;
    col.friction    = 0.6f;
    scene.GetRegistry().add<Collider>(e, col);
}

void TableSetup::CreateCushions(Scene& scene) {
    // 쿠션 색 (진한 초록)
    std::vector<Color> cpix(4 * 4, Color(20, 70, 20, 255));
    m_cushionTex = Texture::FromPixels(4, 4, cpix);  // 단일 텍스처 공유

    // [0]=뒤  [1]=앞  [2]=좌  [3]=우
    struct CushionDef { Vec3 pos; float w; float d; };
    CushionDef defs[4] = {
        { { 0.0f, CUSHION_H*0.5f, -(TABLE_D*0.5f + CUSHION_T*0.5f) }, TABLE_W + CUSHION_T*2, CUSHION_T },
        { { 0.0f, CUSHION_H*0.5f,  (TABLE_D*0.5f + CUSHION_T*0.5f) }, TABLE_W + CUSHION_T*2, CUSHION_T },
        { {-(TABLE_W*0.5f + CUSHION_T*0.5f), CUSHION_H*0.5f, 0.0f }, CUSHION_T, TABLE_D },
        { { (TABLE_W*0.5f + CUSHION_T*0.5f), CUSHION_H*0.5f, 0.0f }, CUSHION_T, TABLE_D },
    };

    for (int i = 0; i < 4; ++i) {
        m_cushionMesh[i] = MeshGenerator::CreateBox(defs[i].w, CUSHION_H, defs[i].d);

        Entity e = scene.CreateEntity();
        Transform tf;
        tf.localPos = defs[i].pos;
        scene.GetRegistry().add<Transform>(e, tf);

        MeshRenderer mr;
        mr.mesh = &m_cushionMesh[i];
        mr.material.albedo = &m_cushionTex;
        scene.GetRegistry().add<MeshRenderer>(e, mr);

        Collider col;
        col.shape       = ColliderShape::AABB;
        col.halfExtents = {defs[i].w * 0.5f, CUSHION_H * 0.5f, defs[i].d * 0.5f};
        col.restitution = 0.75f;
        col.friction    = 0.1f;
        scene.GetRegistry().add<Collider>(e, col);
    }
}

void TableSetup::CreatePockets(Scene& scene) {
    m_pocketMesh = MeshGenerator::CreateSphere(8, 8, 0.32f);

    // 포켓 위치: 모서리 4 + 장변 중앙 2
    Vec3 positions[6] = {
        {-(TABLE_W*0.5f), 0.0f, -(TABLE_D*0.5f)},  // 좌뒤
        { (TABLE_W*0.5f), 0.0f, -(TABLE_D*0.5f)},  // 우뒤
        {-(TABLE_W*0.5f), 0.0f,  (TABLE_D*0.5f)},  // 좌앞
        { (TABLE_W*0.5f), 0.0f,  (TABLE_D*0.5f)},  // 우앞
        {          0.0f,  0.0f, -(TABLE_D*0.5f)},  // 중앙뒤
        {          0.0f,  0.0f,  (TABLE_D*0.5f)},  // 중앙앞
    };

    for (int i = 0; i < 6; ++i) {
        Entity e = scene.CreateEntity();
        Transform tf;
        tf.localPos = positions[i];
        scene.GetRegistry().add<Transform>(e, tf);

        Collider col;
        col.shape     = ColliderShape::Sphere;
        col.radius    = 0.32f;
        col.isTrigger = true;
        scene.GetRegistry().add<Collider>(e, col);
    }
}
```

- [ ] **Step 3: BilliardsScene에 TableSetup 연동**

`BilliardsScene.h` private 섹션에 추가:
```cpp
#include "game/billiards/TableSetup.h"
// ...
private:
    TableSetup m_tableSetup;
```

`BilliardsScene.cpp` OnEnter에서 시스템 등록 전에 추가:
```cpp
m_tableSetup.Init(m_scene);
```

- [ ] **Step 4: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: 초록 당구대와 쿠션이 쿼터뷰로 보임.

---

## Task 4: BallManager — 공 배치

**Files:**
- Create: `src/game/billiards/BallManager.h`
- Create: `src/game/billiards/BallManager.cpp`
- Modify: `src/game/billiards/BilliardsScene.h/.cpp`

- [ ] **Step 1: BallManager.h 생성**

```cpp
#pragma once
#include "ecs/Entity.hpp"
#include "scene/Scene.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"
#include <vector>

class BallManager {
public:
    void Init(Scene& scene);
    void Reset(Scene& scene);           // R키 리셋

    Entity GetCueBall() const { return m_cueBall; }
    const std::vector<Entity>& GetNumberBalls() const { return m_numberBalls; }
    void RemoveBall(Scene& scene, Entity e);

    static constexpr float BALL_R = 0.25f;
    static constexpr Vec3  CUE_START = {-3.0f, BALL_R, 0.0f};
    static constexpr Vec3  RACK_TIP  = { 2.0f, BALL_R, 0.0f};

private:
    Entity CreateBall(Scene& scene, const Vec3& pos, const Vec3& tint);
    void   PlaceRack(Scene& scene);

    Entity m_cueBall = NULL_ENTITY;
    std::vector<Entity> m_numberBalls;

    Mesh    m_ballMesh;
    Texture m_ballTex[16];   // 0=큐볼, 1~15=번호공
};
```

- [ ] **Step 2: BallManager.cpp — CreateBall 헬퍼**

```cpp
#include "game/billiards/BallManager.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "renderer/MeshRenderer.h"
#include "resource/MeshGenerator.h"
#include "scene/Transform.h"
#include "core/Color.h"
#include <cmath>

// 번호공 색상 (인덱스 0=큐볼, 1~15)
static const Vec3 BALL_COLORS[16] = {
    {1.0f, 1.0f, 1.0f},   // 0 큐볼 — 흰색
    {1.0f, 0.9f, 0.0f},   // 1 노랑
    {0.1f, 0.2f, 0.8f},   // 2 파랑
    {0.8f, 0.1f, 0.1f},   // 3 빨강
    {0.5f, 0.1f, 0.5f},   // 4 보라
    {0.9f, 0.4f, 0.1f},   // 5 주황
    {0.1f, 0.5f, 0.1f},   // 6 초록
    {0.6f, 0.1f, 0.1f},   // 7 어두운빨강
    {0.05f,0.05f,0.05f},  // 8 검정
    {1.0f, 0.9f, 0.0f},   // 9  (줄무늬 근사 — 동일 색 사용)
    {0.1f, 0.2f, 0.8f},   // 10
    {0.8f, 0.1f, 0.1f},   // 11
    {0.5f, 0.1f, 0.5f},   // 12
    {0.9f, 0.4f, 0.1f},   // 13
    {0.1f, 0.5f, 0.1f},   // 14
    {0.6f, 0.1f, 0.1f},   // 15
};

void BallManager::Init(Scene& scene) {
    m_ballMesh = MeshGenerator::CreateSphere(16, 16, BALL_R);

    // 큐볼
    std::vector<Color> whitePx(4*4, Color(255,255,255,255));
    m_ballTex[0] = Texture::FromPixels(4,4,whitePx);
    m_cueBall = CreateBall(scene, CUE_START, BALL_COLORS[0]);

    PlaceRack(scene);
}

Entity BallManager::CreateBall(Scene& scene, const Vec3& pos, const Vec3& tint) {
    Entity e = scene.CreateEntity();

    Transform tf;
    tf.localPos   = pos;
    tf.localScale = {1.0f, 1.0f, 1.0f};
    scene.GetRegistry().add<Transform>(e, tf);

    MeshRenderer mr;
    mr.mesh = &m_ballMesh;
    mr.material.tint = tint;
    scene.GetRegistry().add<MeshRenderer>(e, mr);

    Collider col;
    col.shape       = ColliderShape::Sphere;
    col.radius      = BALL_R;
    col.restitution = 0.8f;
    col.friction    = 0.3f;
    scene.GetRegistry().add<Collider>(e, col);

    RigidBody rb;
    rb.mass        = 1.0f;
    rb.drag        = 0.4f;    // BilliardPhysicsSystem이 롤링 마찰 담당하므로 낮게 설정
    rb.angularDrag = 0.5f;
    rb.useGravity  = true;
    rb.isKinematic = false;
    scene.GetRegistry().add<RigidBody>(e, rb);

    return e;
}
```

- [ ] **Step 3: BallManager.cpp — PlaceRack (정삼각형 랙)**

```cpp
void BallManager::PlaceRack(Scene& scene) {
    // 정삼각형 랙: 행 1,2,3,4,5 = 공 1,2,3,4,5개 (총 15개)
    // spacing: 공 지름 + 약간의 여유
    const float spacing = BALL_R * 2.0f + 0.01f;
    const float rowDZ   = spacing;
    const float rowDX   = spacing * std::sqrt(3.0f) * 0.5f;

    // 번호 배치: 1번 맨앞, 8번 중앙(행3 중간)
    int order[15] = {1, 2,9, 3,8,10, 4,14,13,11, 5,6,7,15,12};
    int idx = 0;

    for (int row = 0; row < 5; ++row) {
        int count = row + 1;
        float startZ = -(count - 1) * rowDZ * 0.5f;
        for (int col = 0; col < count; ++col) {
            Vec3 pos = RACK_TIP;
            pos.x += row * rowDX;
            pos.z += startZ + col * rowDZ;

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
        return;
    }
    auto it = std::find(m_numberBalls.begin(), m_numberBalls.end(), e);
    if (it != m_numberBalls.end()) m_numberBalls.erase(it);
}

void BallManager::Reset(Scene& scene) {
    if (m_cueBall != NULL_ENTITY) scene.DestroyEntity(m_cueBall);
    for (Entity e : m_numberBalls) scene.DestroyEntity(e);
    m_numberBalls.clear();
    m_cueBall = NULL_ENTITY;
    Init(scene);
}
```

- [ ] **Step 4: BilliardsScene에 BallManager 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/BallManager.h"
// private:
    BallManager m_ballManager;
```

`BilliardsScene.cpp` OnEnter에 TableSetup 호출 다음에 추가:
```cpp
m_ballManager.Init(m_scene);
```

- [ ] **Step 5: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: 테이블 위에 흰 큐볼과 15개 번호공이 정삼각형으로 배치됨. 중력으로 테이블에 안착.

---

## Task 5: CueController — 조준선

**Files:**
- Create: `src/game/billiards/CueController.h`
- Create: `src/game/billiards/CueController.cpp`
- Modify: `src/game/billiards/BilliardsScene.h/.cpp`

- [ ] **Step 1: CueController.h 생성**

```cpp
#pragma once
#include "ecs/Entity.hpp"
#include "scene/Scene.h"
#include "renderer/Renderer.h"
#include "math/Vec3.h"
#include "math/Vec2.h"
#include "game/billiards/GameTypes.h"

class BallManager;

class CueController {
public:
    void Init(BallManager* ballMgr);
    void Update(float dt, Scene& scene, Renderer& renderer, GameState& state);

    Vec3  GetAimDir()  const { return m_aimDir; }
    float GetPower()   const { return m_power; }
    Vec2  GetImpact()  const { return m_impactOffset; }
    void  SetImpact(Vec2 offset) { m_impactOffset = offset; }

    static constexpr float AIM_SPEED  = 90.0f;   // 도/초
    static constexpr float MAX_SPEED  = 18.0f;   // 최대 발사 속력
    static constexpr float MAX_CHARGE = 2.0f;    // 최대 충전 시간(초)
    static constexpr float SPIN_SCALE = 8.0f;    // 임팩트 오프셋 → angularVelocity 배율

private:
    void DrawAimLine(Renderer& renderer, const Vec3& cueBallPos);

    BallManager* m_ballMgr = nullptr;
    float m_aimAngle   = 180.0f;   // 도 단위
    Vec3  m_aimDir     = {-1,0,0};
    float m_chargeTime = 0.0f;
    float m_power      = 0.0f;
    bool  m_wasDown    = false;
    Vec2  m_impactOffset = {0.0f, 0.0f};
};
```

- [ ] **Step 2: CueController.cpp 생성**

```cpp
#include "game/billiards/CueController.h"
#include "game/billiards/BallManager.h"
#include "input/InputManager.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
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

    // A/D 조준각 회전
    if (input.IsKeyDown(KeyCode::A)) m_aimAngle += AIM_SPEED * dt;
    if (input.IsKeyDown(KeyCode::D)) m_aimAngle -= AIM_SPEED * dt;

    m_aimDir = Vec3{
        std::cos(m_aimAngle * DEG2RAD),
        0.0f,
        std::sin(m_aimAngle * DEG2RAD)
    };

    Vec3 cueBallPos = reg.get<Transform>(cueBall).localPos;

    // 파워 충전
    bool isDown = input.IsMouseDown(MouseButton::Left);
    if (isDown) {
        m_chargeTime += dt;
        if (m_chargeTime > MAX_CHARGE) m_chargeTime = MAX_CHARGE;
    }
    m_power = m_chargeTime / MAX_CHARGE;

    // 발사 — 이전 프레임 누름 + 현재 프레임 안누름
    if (m_wasDown && !isDown && m_chargeTime > 0.01f) {
        auto& rb = reg.get<RigidBody>(cueBall);
        rb.velocity = m_aimDir * (m_power * MAX_SPEED);
        // 스핀 적용
        rb.angularVelocity.z += m_impactOffset.y * SPIN_SCALE;
        rb.angularVelocity.x -= m_impactOffset.x * SPIN_SCALE;
        m_chargeTime = 0.0f;
        m_power      = 0.0f;
        state        = GameState::Shooting;
    }
    m_wasDown = isDown;

    // 조준선 그리기
    DrawAimLine(renderer, cueBallPos);
}

void CueController::DrawAimLine(Renderer& renderer, const Vec3& cueBallPos) {
    Vec3 end = cueBallPos + m_aimDir * 6.0f;
    renderer.DrawLine(cueBallPos, end, {1.0f, 1.0f, 0.0f});
}
```

- [ ] **Step 3: BilliardsScene에 CueController 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/CueController.h"
// private:
    CueController m_cueController;
```

`BilliardsScene.cpp` OnEnter 끝에 추가:
```cpp
m_cueController.Init(&m_ballManager);
```

`OnUpdate`에서 `m_world.update(dt)` 다음에 추가:
```cpp
m_cueController.Update(dt, m_scene, m_renderer, m_state);
```

- [ ] **Step 4: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: A/D 키로 노란 조준선 회전. 좌클릭 hold 후 떼면 큐볼 발사.

---

## Task 6: BilliardPhysicsSystem — 롤링 마찰 + 스핀

**Files:**
- Create: `src/game/billiards/BilliardPhysicsSystem.h`
- Create: `src/game/billiards/BilliardPhysicsSystem.cpp`
- Modify: `src/game/billiards/BilliardsScene.cpp`

- [ ] **Step 1: BilliardPhysicsSystem.h 생성**

```cpp
#pragma once
#include "ecs/Registry.hpp"

class BilliardPhysicsSystem {
public:
    void Update(Registry& reg, float dt);

    static constexpr float BALL_RADIUS   = 0.25f;
    static constexpr float FRICTION_COEF = 0.2f;   // 슬라이딩 마찰 계수
    static constexpr float ROLL_RESIST   = 0.015f;  // 순수 굴림 저항
    static constexpr float GRAVITY       = 9.8f;
    static constexpr float SLIDE_THRESH  = 0.02f;   // 슬라이딩 판정 임계값
};
```

- [ ] **Step 2: BilliardPhysicsSystem.cpp 생성**

```cpp
#include "game/billiards/BilliardPhysicsSystem.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
#include <cmath>

void BilliardPhysicsSystem::Update(Registry& reg, float dt) {
    // RigidBody + Collider + Transform 모두 가진 엔티티만 처리
    // view<A,B,C>()의 iterator는 tuple<A&,B&,C&>를 반환 (entity 없음)
    for (auto [rb, col, tf] : reg.view<RigidBody, Collider, Transform>()) {
        if (rb.isKinematic) continue;
        if (col.shape != ColliderShape::Sphere) continue;

        float r = col.radius;

        // 테이블 위에 있는지 판정 (y ≈ r 이면 바닥 접촉 중)
        bool onTable = std::abs(tf.localPos.y - r) < 0.05f;
        if (!onTable) continue;

        // 접촉점 속도: v_contact = v - (ω × r) — y축 무시, xz 평면만
        // ω.z → x방향 기여, ω.x → z방향 기여
        Vec3 rollVel = {
            -rb.angularVelocity.z * r,
             0.0f,
             rb.angularVelocity.x * r
        };
        Vec3 slipVel = {
            rb.velocity.x - rollVel.x,
            0.0f,
            rb.velocity.z - rollVel.z
        };
        float slipLen = std::sqrt(slipVel.x*slipVel.x + slipVel.z*slipVel.z);

        if (slipLen > SLIDE_THRESH) {
            // 슬라이딩: 마찰력으로 v 감속 + ω 보정
            float fAcc = FRICTION_COEF * GRAVITY;
            Vec3 frDir = { -slipVel.x / slipLen, 0.0f, -slipVel.z / slipLen };

            rb.velocity.x += frDir.x * fAcc * dt;
            rb.velocity.z += frDir.z * fAcc * dt;

            // 각속도 보정: τ = F×r / I,  I = 2/5 m r²  →  α = 5/2 * F/(m*r)
            float angAcc = 2.5f * fAcc / r;
            rb.angularVelocity.z -= frDir.x * angAcc * dt;
            rb.angularVelocity.x += frDir.z * angAcc * dt;
        } else {
            // 순수 굴림: 굴림 저항으로 함께 감속
            float decay = 1.0f - ROLL_RESIST * dt * 60.0f;
            if (decay < 0.0f) decay = 0.0f;
            rb.velocity.x     *= decay;
            rb.velocity.z     *= decay;
            rb.angularVelocity.x *= decay;
            rb.angularVelocity.z *= decay;
        }
    }
}
```

- [ ] **Step 3: BilliardsScene에 BilliardPhysicsSystem 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/BilliardPhysicsSystem.h"
// private:
    BilliardPhysicsSystem m_billiardPhysics;
```

`BilliardsScene.cpp` OnFixedUpdate를 다음으로 교체:
```cpp
void BilliardsScene::OnFixedUpdate() {
    m_world.fixed_update(1.0f / 60.0f);
    m_billiardPhysics.Update(m_scene.GetRegistry(), 1.0f / 60.0f);
}
```

- [ ] **Step 4: Shooting → Aiming 전환 로직 추가**

`BilliardsScene.cpp` OnUpdate에 추가:
```cpp
// 모든 공이 멈추면 Aiming으로 복귀
// pool<T>()는 컴포넌트만 직접 이터레이션 (entity 없음)
if (m_state == GameState::Shooting) {
    bool allStopped = true;
    for (auto& rb : m_scene.GetRegistry().pool<RigidBody>()) {
        float vLen = std::sqrt(rb.velocity.x*rb.velocity.x +
                               rb.velocity.y*rb.velocity.y +
                               rb.velocity.z*rb.velocity.z);
        float wLen = std::sqrt(rb.angularVelocity.x*rb.angularVelocity.x +
                               rb.angularVelocity.z*rb.angularVelocity.z);
        if (vLen > 0.05f || wLen > 0.05f) { allStopped = false; break; }
    }
    if (allStopped) m_state = GameState::Aiming;
}
```

- [ ] **Step 5: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: 큐볼 발사 후 자연스럽게 감속. 백스핀 시 충돌 후 뒤로 밀려남.

---

## Task 7: PocketSystem — 포켓 감지 & 큐볼 복귀

**Files:**
- Create: `src/game/billiards/PocketSystem.h`
- Create: `src/game/billiards/PocketSystem.cpp`
- Modify: `src/game/billiards/BilliardsScene.h/.cpp`

- [ ] **Step 1: PocketSystem.h 생성**

```cpp
#pragma once
#include "ecs/Entity.hpp"
#include "scene/Scene.h"
#include "math/Vec3.h"
#include <queue>

class BallManager;

class PocketSystem {
public:
    void Init(Scene& scene, BallManager* ballMgr);
    void Update(Scene& scene, GameState& state);   // 큐볼 복귀 처리

private:
    BallManager* m_ballMgr = nullptr;
    std::queue<Entity> m_toRemove;   // 이벤트 콜백에서 직접 제거 금지 — 큐에 쌓고 Update에서 처리
    bool m_cueBallPocketed = false;
};
```

- [ ] **Step 2: PocketSystem.cpp 생성**

```cpp
#include "game/billiards/PocketSystem.h"
#include "game/billiards/BallManager.h"
#include "game/billiards/BilliardsScene.h"
#include "event/EventBus.h"
#include "event/Events.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"

void PocketSystem::Init(Scene& scene, BallManager* ballMgr) {
    m_ballMgr = ballMgr;

    EventBus::Subscribe<TriggerEnterEvent>([this](const TriggerEnterEvent& e) {
        // e.other = 포켓에 들어간 공 Entity
        m_toRemove.push(e.other);
        if (e.other == m_ballMgr->GetCueBall()) {
            m_cueBallPocketed = true;
        }
    });
}

void PocketSystem::Update(Scene& scene, GameState& state) {
    while (!m_toRemove.empty()) {
        Entity e = m_toRemove.front();
        m_toRemove.pop();

        if (e == m_ballMgr->GetCueBall() && m_cueBallPocketed) {
            // 큐볼 파울: 초기 위치로 복귀
            auto& tf = scene.GetRegistry().get<Transform>(e);
            tf.localPos = BallManager::CUE_START;
            auto& rb = scene.GetRegistry().get<RigidBody>(e);
            rb.velocity        = {0,0,0};
            rb.angularVelocity = {0,0,0};
            m_cueBallPocketed  = false;
            state = GameState::Aiming;
        } else {
            m_ballMgr->RemoveBall(scene, e);
        }
    }
}
```

- [ ] **Step 3: BilliardsScene에 PocketSystem 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/PocketSystem.h"
// private:
    PocketSystem m_pocketSystem;
```

`BilliardsScene.cpp` OnEnter 끝에 추가:
```cpp
m_pocketSystem.Init(m_scene, &m_ballManager);
```

`OnUpdate`에 추가:
```cpp
m_pocketSystem.Update(m_scene, m_state);
```

- [ ] **Step 4: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: 공이 포켓에 들어가면 사라짐. 큐볼이 포켓에 들어가면 제자리 복귀.

---

## Task 8: SpinDialUI — 임팩트 포인트 다이얼

**Files:**
- Create: `src/game/billiards/SpinDialUI.h`
- Create: `src/game/billiards/SpinDialUI.cpp`
- Modify: `src/game/billiards/BilliardsScene.h/.cpp`

- [ ] **Step 1: SpinDialUI.h 생성**

```cpp
#pragma once
#include "ui/UIRenderer.h"
#include "math/Vec2.h"
#include "game/billiards/GameTypes.h"

class SpinDialUI {
public:
    void Render(UIRenderer& ui, int screenW, int screenH, bool aiming);
    void Update(int screenW, int screenH);   // 마우스 입력

    Vec2 GetOffset() const { return m_offset; }  // (-1~1, -1~1)

    static constexpr float DIAL_X      = 90.0f;   // 화면 좌하단 기준 x
    static constexpr float DIAL_Y_BOT  = 90.0f;   // 화면 하단에서 위로
    static constexpr float DIAL_R      = 40.0f;   // 다이얼 반지름 (픽셀)

private:
    Vec2 m_offset = {0.0f, 0.0f};
};
```

- [ ] **Step 2: SpinDialUI.cpp 생성**

```cpp
#include "game/billiards/SpinDialUI.h"
#include "input/InputManager.h"
#include <cmath>

void SpinDialUI::Update(int screenW, int screenH) {
    auto& input = InputManager::Get();
    if (!input.IsMouseDown(MouseButton::Right)) return;

    float cx = DIAL_X;
    float cy = screenH - DIAL_Y_BOT;
    float mx = input.MouseX();
    float my = input.MouseY();

    float dx = mx - cx;
    float dy = my - cy;
    float dist = std::sqrt(dx*dx + dy*dy);

    if (dist < DIAL_R) {
        m_offset = { dx / DIAL_R, -dy / DIAL_R };   // y 반전 (화면좌표 → 게임좌표)
    } else if (dist > 0.001f) {
        // 원 경계에 클램프
        m_offset = { dx / dist, -dy / dist };
    }
}

void SpinDialUI::Render(UIRenderer& ui, int screenW, int screenH, bool aiming) {
    if (!aiming) return;

    float cx = DIAL_X;
    float cy = screenH - DIAL_Y_BOT;
    float r  = DIAL_R;

    // 다이얼 배경 원 — 사각형으로 근사 (엔진 DrawRect)
    ui.DrawRect(cx - r, cy - r, r*2, r*2, {0.1f, 0.1f, 0.1f}, 0.7f);

    // 십자선
    ui.DrawRect(cx - r, cy - 0.5f, r*2, 1.0f, {0.3f,0.3f,0.3f}, 0.8f);
    ui.DrawRect(cx - 0.5f, cy - r, 1.0f, r*2, {0.3f,0.3f,0.3f}, 0.8f);

    // 임팩트 점
    float dotX = cx + m_offset.x * r;
    float dotY = cy - m_offset.y * r;
    ui.DrawRect(dotX - 4, dotY - 4, 8, 8, {1.0f, 0.9f, 0.0f}, 1.0f);

    // 레이블
    ui.DrawText(cx - r, cy + r + 6, "SPIN", 12, {0.7f, 0.7f, 0.7f}, 0.8f);
}
```

- [ ] **Step 3: BilliardsScene에 SpinDialUI 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/SpinDialUI.h"
// private:
    SpinDialUI m_spinDial;
```

`OnUpdate`에 추가:
```cpp
auto& win = m_app->GetWindow();
m_spinDial.Update(win.PixelWidth(), win.PixelHeight());
m_cueController.SetImpact(m_spinDial.GetOffset());
```

`OnRender` 직전(`SwapBuffers` 전)에 추가:
```cpp
// UI 렌더링은 world.update 이후, SwapBuffers 이전에
auto& win = m_app->GetWindow();
m_uiRenderer.BeginFrame(win.PixelWidth(), win.PixelHeight());
m_spinDial.Render(m_uiRenderer, win.PixelWidth(), win.PixelHeight(),
                  m_state == GameState::Aiming);
m_uiRenderer.EndFrame();
```

- [ ] **Step 4: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과: 좌하단에 검정 원 다이얼과 노란 점 표시. 우클릭 드래그로 점 이동. 점 위치에 따라 큐볼 발사 후 스핀 방향이 달라짐.

---

## Task 9: BilliardsUI — 미니멀 HUD + R키 리셋

**Files:**
- Create: `src/game/billiards/BilliardsUI.h`
- Create: `src/game/billiards/BilliardsUI.cpp`
- Modify: `src/game/billiards/BilliardsScene.h/.cpp`

- [ ] **Step 1: BilliardsUI.h 생성**

```cpp
#pragma once
#include "ui/UIRenderer.h"
#include "game/billiards/GameTypes.h"
#include <string>

class BilliardsUI {
public:
    void ShowFoul(float duration = 2.0f);
    void Update(float dt);
    void Render(UIRenderer& ui, int w, int h, GameState state,
                float power, int ballsRemaining);

private:
    float m_foulTimer = 0.0f;
};
```

- [ ] **Step 2: BilliardsUI.cpp 생성**

```cpp
#include "game/billiards/BilliardsUI.h"

void BilliardsUI::ShowFoul(float duration) {
    m_foulTimer = duration;
}

void BilliardsUI::Update(float dt) {
    if (m_foulTimer > 0.0f) m_foulTimer -= dt;
}

void BilliardsUI::Render(UIRenderer& ui, int w, int h, GameState state,
                         float power, int ballsRemaining) {
    // 상태 텍스트 — 좌상단
    const char* stateStr = "AIMING";
    if (state == GameState::Shooting)  stateStr = "SHOOTING";
    if (state == GameState::Resetting) stateStr = "FOUL";
    ui.DrawText(16, 16, stateStr, 18, {1.0f, 1.0f, 1.0f}, 0.9f);

    // 남은 공 수 — 좌상단 두 번째 줄
    ui.DrawText(16, 40, "Balls: " + std::to_string(ballsRemaining), 14,
                {0.8f, 0.8f, 0.8f}, 0.8f);

    // 파워 게이지 — 우하단
    float barW = 120.0f;
    float barH = 12.0f;
    float barX = w - barW - 16.0f;
    float barY = h - 40.0f;
    ui.DrawRect(barX, barY, barW, barH, {0.2f, 0.2f, 0.2f}, 0.7f);
    ui.DrawRect(barX, barY, barW * power, barH, {1.0f, 0.5f, 0.0f}, 0.9f);
    ui.DrawText(barX, barY - 18, "POWER", 12, {0.7f, 0.7f, 0.7f}, 0.8f);

    // 파울 메시지 — 화면 중앙
    if (m_foulTimer > 0.0f) {
        ui.DrawText(w * 0.5f - 30, h * 0.5f, "FOUL!", 24,
                    {1.0f, 0.3f, 0.3f}, m_foulTimer);
    }

    // R키 안내 — 우상단
    ui.DrawText(w - 100.0f, 16, "[R] Reset", 12, {0.5f, 0.5f, 0.5f}, 0.7f);
}
```

- [ ] **Step 3: BilliardsScene에 BilliardsUI + R키 리셋 연동**

`BilliardsScene.h`에 추가:
```cpp
#include "game/billiards/BilliardsUI.h"
// private:
    BilliardsUI m_billiardsUI;
```

`OnUpdate`에 추가:
```cpp
// R키 리셋
if (InputManager::Get().JustPressed(KeyCode::R)) {
    m_ballManager.Reset(m_scene);
    m_state = GameState::Aiming;
}

m_billiardsUI.Update(dt);
```

`OnRender`의 UI 렌더링 블록을 확장:
```cpp
m_uiRenderer.BeginFrame(win.PixelWidth(), win.PixelHeight());
m_spinDial.Render(m_uiRenderer, win.PixelWidth(), win.PixelHeight(),
                  m_state == GameState::Aiming);
m_billiardsUI.Render(m_uiRenderer, win.PixelWidth(), win.PixelHeight(),
                     m_state, m_cueController.GetPower(),
                     (int)m_ballManager.GetNumberBalls().size());
m_uiRenderer.EndFrame();
```

`PocketSystem`에서 큐볼 파울 발생 시 `m_billiardsUI.ShowFoul()` 호출이 필요. `PocketSystem::Update`에 `BilliardsUI*` 포인터 파라미터 추가:
```cpp
// PocketSystem.h Update 시그니처 변경
void Update(Scene& scene, GameState& state, BilliardsUI* ui);

// PocketSystem.cpp 큐볼 복귀 블록에 추가
if (ui) ui->ShowFoul();
```

`BilliardsScene.cpp` OnUpdate에서 호출 변경:
```cpp
m_pocketSystem.Update(m_scene, m_state, &m_billiardsUI);
```

- [ ] **Step 4: 빌드 & 실행 확인**

```bash
make -j4 && ./bin/Cpp_Engine
```

기대 결과:
- 좌상단에 `AIMING` / `SHOOTING` 상태 텍스트
- 우하단에 주황색 파워 게이지 (좌클릭 hold 시 채워짐)
- 큐볼 포켓 시 `FOUL!` 메시지 2초 표시
- R키로 공 재배치

---

## 빌드 명령 참조

```bash
# 엔진 소스가 CppPool/에 복사된 후
mkdir -p /Users/deepfine/C++Project/CppPool/build
cd /Users/deepfine/C++Project/CppPool/build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
./bin/Cpp_Engine
```
