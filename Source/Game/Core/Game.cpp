#include <chrono>
#include <iostream>
#include <thread>

#include "Device.h"
#include "RenderManager.h"
#include "Window.h"
#include "Stage.h"
#include "DateTime.h"
#include "Time.h"

#include "tracy/Tracy.hpp"

#include <windows.h>

int main()
{
    Window window;

    RenderManager renderingManager;
    renderingManager.Setup();
    std::thread renderingThread(RunRenderThread, &renderingManager);

    Time::Initialize();
    StageManager::Initialize();

    while (Window::windowRunning)
    {
        Time::Update();

        window.Process();
        Device::Get()->StartFrame();
        renderingManager.canStartRenderingFrame = true;
        renderingManager.isFinishedRenderingFrame = false;

        StageManager::Update();

        while (!renderingManager.isFinishedRenderingFrame)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        Device::Get()->EndFrame();

        FrameMark;
    }

    renderingManager.isRendererRunning = false;
    renderingThread.join();
}
