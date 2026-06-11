#pragma once
#include "app/Application.h"

class BilliardsApp : public Application {
public:
    BilliardsApp(int width, int height, const char* title);
protected:
    void OnInit() override;
};
