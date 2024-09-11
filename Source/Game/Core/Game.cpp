#include <iostream>
#include "Device.h"
#include "RenderManager.h"
#include "Window.h"
#include "Stage.h"
#include "Time.h"
#include <windows.h>

int main()
{
    Window window;

    RenderManager renderingManager;
    renderingManager.Setup();

    Time::Initialize();
    StageManager::Initialize();

    while (Window::windowRunning)
    {
        Time::Update();

        window.Process();
        Device::Get()->StartFrame();
        renderingManager.StartFrame();

        StageManager::Update();

        renderingManager.EndFrame();
        Device::Get()->EndFrame();
    }
}
