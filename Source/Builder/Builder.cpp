#include "BuildSystem.h"
#include "BuilderTab.h"
#include "Views/BuildDashboard.h"
#include "Views/FileStatuses.h"
#include "Views/TotalOutput.h"

#include "Device.h"
#include "ImGuiDevice.h"
#include "Window.h"

#include "RenderManager.h"

#include "Symbol Search/SymbolSearch.h"

int main()
{
	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;

	BuildSystem::Get()->Init();

	BuilderTabSystem::Get()->AddTab(new BuildDashboard());
	BuilderTabSystem::Get()->AddTab(new FileStatuses());
	BuilderTabSystem::Get()->AddTab(new TotalOutput());

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		BuildSystem::Get()->UpdateBuild();

		BuilderTabSystem::Get()->RenderTabs();

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}