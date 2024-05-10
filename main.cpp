#include <Windows.h>
#include <iostream>
#include <memory>

#include "Engine.h"

int main() {
    jRenderer::Engine app;

    if (!app.Initialize()) {
        std::cout << "Init failed" << std::endl;
        return -1;
    }

    return app.Run();
}