#include <chrono>
#include <iostream>
#include <thread>

#include "Device.h"
#include "RenderManager.h"
#include "Window.h"
#include "Stage.h"
#include "DateTime.h"
#include "Time.h"

#include <windows.h>

int main()
{
    Window window;

    RenderManager renderingManager;
    renderingManager.Setup();
    std::thread renderingThread(RunRenderThread, &renderingManager);

    Time::Initialize();
    StageManager::Initialize();

    DateTime lastFrameStartTime = DateTime::Now();

    while (Window::windowRunning)
    {
        lastFrameStartTime = DateTime::Now();
        Time::Update();

        window.Process();
        Device::Get()->StartFrame();
        renderingManager.canStartRenderingFrame = true;
        renderingManager.isFinishedRenderingFrame = false;

        StageManager::Update();

        DateTime currentFrameTime = DateTime::Now();
        uint64_t renderTimeMicroseconds = currentFrameTime.GetTicks() - lastFrameStartTime.GetTicks();
        lastFrameStartTime = currentFrameTime;

        std::cout << "Game Time: " << renderTimeMicroseconds << " microseconds" << std::endl;

        while (!renderingManager.isFinishedRenderingFrame)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        lastFrameStartTime = DateTime::Now();
        Device::Get()->EndFrame();
    }

    renderingManager.isRendererRunning = false;
    renderingThread.join();
}
