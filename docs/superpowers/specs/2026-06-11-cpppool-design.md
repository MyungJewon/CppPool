# CppPool Design Spec

Date: 2026-06-11

## Overview

Cpp_Engine v0.5.0 기반 솔로 자유 연습 포켓볼 게임.
승패/점수 없이 물리 기반 당구 시뮬레이션에 집중.
공의 임팩트 포인트에 따른 스핀 물리(롤링 마찰 모델)가 핵심 기능.

---

## 프로젝트 구조

엔진 소스 전체를 CppPool 레포에 복사(일체형). 게임 코드는 `src/game/billiards/`에 추가.
엔진 원본 파일은 수정하지 않음. `main.cpp`만 `BilliardsApp` 사용으로 교체.

```
CppPool/
├── src/
│   ├── app/ ecs/ renderer/ ...     ← Cpp_Engine 원본 (수정 없음)
│   ├── main.cpp                    ← BilliardsApp 사용으로 교체
│   └── game/billiards/
│       ├── BilliardsApp.h/.cpp
│       ├── BilliardsScene.h/.cpp
│       ├── TableSetup.h/.cpp
│       ├── BallManager.h/.cpp
│       ├── CueController.h/.cpp
│       ├── SpinDialUI.h/.cpp
│       ├── BilliardPhysicsSystem.h/.cpp
│       ├── PocketSystem.h/.cpp
│       └── BilliardsUI.h/.cpp
├── CMakeLists.txt
└── assets/
```

---

## 각 컴포넌트 책임

| 파일 | 책임 |
|---|---|
| `BilliardsApp` | Application 진입점, 씬 등록 |
| `BilliardsScene` | IScene 구현, 시스템 조율, 게임 상태 관리 |
| `TableSetup` | 테이블 상판, 쿠션 4면, 포켓 6개 Entity 생성 |
| `BallManager` | 공 16개 생성·관리, 랙 배치, 제거·리셋 |
| `CueController` | A/D 조준각, 좌클릭 파워 충전, 발사 처리 |
| `SpinDialUI` | 임팩트 포인트 HUD 다이얼, 마우스로 점 이동 |
| `BilliardPhysicsSystem` | 롤링 마찰 모델, 스핀→선속도 변환, 쿠션 사이드스핀 보정 |
| `PocketSystem` | TriggerEnterEvent 수신, 공 제거, 큐볼 파울 복귀 |
| `BilliardsUI` | 미니멀 오버레이 HUD (현재 상태, 파워 게이지, 파울 메시지) |

---

## 카메라

쿼터뷰 고정 시점. 탭 카메라 전환 비활성화.

```cpp
cam.eye    = {0.0f, 7.0f, 5.0f};
cam.target = {0.0f, 0.0f, 0.0f};
cam.fov    = 60.0f;
```

---

## 테이블 스펙

- 상판: 9 × 4.5 단위, y=0
- 쿠션 4면: CreateBox, Collider(AABB), RigidBody 없음(static)
- 포켓 6개: 모서리 4 + 장변 중앙 2, radius=0.32f, isTrigger=true

---

## 공 스펙

```cpp
// RigidBody
rb.mass = 1.0f;  rb.drag = 0.6f;  rb.angularDrag = 2.0f;  rb.useGravity = true;

// Collider
col.shape = ColliderShape::Sphere;  col.radius = 0.25f;
col.restitution = 0.8f;  col.friction = 0.3f;
```

- 큐볼: `{-3.0f, 0.25f, 0.0f}`
- 번호공 1~15: 정삼각형 랙, 꼭짓점 `{2.0f, 0.25f, 0.0f}`

---

## 조준 & 발사

- **조준**: A/D 키로 `m_aimAngle` 증감 (90도/초 권장)
- **조준선**: `DrawLine(cueBallPos, cueBallPos + aimDir*6, {1,1,0})`
- **파워**: 좌클릭 홀드 → Timer 충전 (최대 2초), 떼면 발사
- **발사**: `rb.velocity = aimDir * (elapsed/2.0f) * maxSpeed`

---

## 스핀 시스템 (BilliardPhysicsSystem)

### 임팩트 포인트 → 초기 angularVelocity

```cpp
// SpinDialUI에서 받은 오프셋 (-1~1 범위)
Vec2 impact = spinDial.GetOffset();
rb.angularVelocity.z += impact.y * spinScale;  // 탑/백스핀
rb.angularVelocity.x += impact.x * spinScale;  // 사이드스핀
```

### 롤링 마찰 모델 (매 fixed_update)

```
순수 굴림 조건: v = ω × r
슬라이딩 중: 마찰력으로 v 감속 + ω를 v 방향으로 가속
조건 달성 후: 굴림 저항으로 함께 감속
```

### 쿠션 사이드스핀 보정

`CollisionEnterEvent` 수신 시 쿠션과의 충돌이면 `angularVelocity.x`에 따라 반사 후 velocity 횡방향 성분 보정.

---

## 게임 상태

```cpp
enum class GameState { Aiming, Shooting, Resetting };
```

- `Aiming`: 조준·다이얼 조작 가능
- `Shooting`: 모든 공 velocity.length() < 0.05f 될 때까지 대기
- `Resetting`: 큐볼 파울 복귀 중 (R키로 전체 리셋)

---

## HUD (BilliardsUI)

미니멀 오버레이 스타일:
- 좌상단: 현재 상태 텍스트 (`AIMING` / `SHOOTING`)
- 우하단: 파워 게이지 바
- 좌하단: `SpinDialUI` 다이얼 (원 + 임팩트 포인트 점)
- 중앙 일시적: 파울 메시지 (2초 후 사라짐)
- R키 안내 텍스트 (우상단)

---

## 구현 순서

```
1단계: 엔진 복사 + 빌드 환경 구성 + BilliardsApp/Scene 뼈대
2단계: 테이블 + 공 배치 + 물리 동작 확인
3단계: 조준선 + 파워 발사 (스핀 없이)
4단계: BilliardPhysicsSystem — 롤링 마찰 + 스핀
5단계: 포켓 감지 + 큐볼 복귀

6단계: HUD + SpinDialUI
```
