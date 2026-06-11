#pragma once

#include "ecs/Entity.hpp"
#include "game/billiards/GameTypes.h"
#include <queue>

class Scene;
class BallManager;
class BilliardsUI;

class PocketSystem {
public:
    ~PocketSystem();

    void Init(Scene& scene, BallManager* ballMgr);
    void Update(Scene& scene, GameState& state, BilliardsUI* ui);

private:
    BallManager* m_ballMgr = nullptr;
    std::queue<Entity> m_toRemove;
    bool m_cueBallPocketed = false;
};
