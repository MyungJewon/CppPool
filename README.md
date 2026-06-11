# CppPool

Cpp_Engine v0.5.0 기반 물리 당구 시뮬레이션. 승패 없이 자유 연습 형태로, 스핀 물리와 쿠션 반사에 집중한 포켓볼 게임.

---

## 빌드

```bash
cmake -B build
cmake --build build
./build/CppPool
```

요구사항: CMake 3.20+, C++17, OpenGL 3.3+

---

## 조작법

| 입력 | 동작 |
|---|---|
| `A` / `D` | 조준 각도 회전 |
| `Q` / `E` | 카메라 궤도 회전 |
| 좌클릭 홀드 | 파워 충전 |
| 좌클릭 릴리즈 | 발사 |
| 우클릭 드래그 | 스핀 임팩트 포인트 설정 |
| `R` | 공 리셋 |

---

## 게임 구조

```
src/game/billiards/
├── BilliardsApp         진입점
├── BilliardsScene       시스템 조율, 게임 상태 관리
├── TableSetup           테이블·쿠션·포켓 Entity 생성
├── BallManager          공 16개 생성·랙 배치·리셋
├── CueController        조준·파워 충전·발사
├── SpinDialUI           스핀 임팩트 포인트 HUD
├── BilliardPhysicsSystem  롤링 마찰·스핀 물리
├── PocketSystem         포켓 감지·공 제거·파울 처리
└── BilliardsUI          상태 오버레이 HUD
```

---

## 물리

- 롤링 마찰: 슬립 속도 기반 → 선속도·각속도 동시 수렴
- 스핀: 임팩트 포인트 오프셋 → `angularVelocity` → 커브·탑/백스핀
- 쿠션 반발: `restitution = 0.75`
- 큐볼 파울: 포켓 인 시 원래 위치로 복귀

---

## 튜닝 파라미터

| 파라미터 | 위치 | 기본값 |
|---|---|---|
| 최대 샷 속도 | `CueController.h` `MAX_SPEED` | 30.0 |
| 스핀 강도 | `CueController.h` `SPIN_SCALE` | 30.0 |
| 쿠션 반발 | `TableSetup.cpp` `restitution` | 0.75 |
| 롤링 저항 | `BilliardPhysicsSystem.h` `ROLL_RESIST` | — |

---

## 플레이영상
https://github.com/user-attachments/assets/aeb89bbe-14bf-48ac-9f8c-63a587da4cb2



