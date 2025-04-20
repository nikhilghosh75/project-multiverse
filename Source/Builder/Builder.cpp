#include "BuildUI.h"

#include "BuildSystem.h"

#include "Device.h"
#include "ImGuiDevice.h"
#include "Window.h"

#include "RenderManager.h"

#include "Symbol Search/SymbolSearch.h"

int main()
{
	BuildUI buildUI;

	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;
	renderingManager.Setup();

	BuildSystem::Get()->Init();

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		BuildSystem::Get()->UpdateBuild();

		buildUI.Render();

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}