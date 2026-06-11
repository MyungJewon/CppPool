#include "game/billiards/BilliardsApp.h"
#include "game/billiards/BilliardsScene.h"
#include "app/IScene.h"
#include <memory>

BilliardsApp::BilliardsApp(int width, int height, const char* title)
    : Application(width, height, title) {}

void BilliardsApp::OnInit() {
    LoadScene(std::unique_ptr<IScene>(std::make_unique<BilliardsScene>()));
}
