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
