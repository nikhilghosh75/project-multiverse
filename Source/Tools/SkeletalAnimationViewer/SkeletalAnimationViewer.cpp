#include "SkeletalAnimationLoader.h"
#include "SkeletalAnimationTab.h"
#include "Importer/UnitySkeletonImporter.h"
#include "Views/LayerHierarchyView.h"
#include "Views/MainView.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "Device.h"
#include "ImGuiDevice.h"
#include "Texture.h"
#include "Window.h"

#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "RenderManager.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include <unordered_map>

int main()
{
	PhotoshopAPI::File inputFile = PhotoshopAPI::File("C:/Users/debgh/OneDrive/Documents/Unity Projects/Dreamwillow/Assets/Art/Characters/Player/PlayerHorizSprite.psb");

	std::unique_ptr<PhotoshopAPI::PhotoshopFile> file = std::make_unique<PhotoshopAPI::PhotoshopFile>();
	PhotoshopAPI::ProgressCallback callback;
	file->read(inputFile, callback);

	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;
	renderingManager.Setup();

	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile = new PhotoshopAPI::LayeredFile<uint8_t>(file);
	SkeletalAnimationLoader::Get()->RegisterPhotoshopFile(layeredFile);

	std::string tempPath = "C:/Users/debgh/OneDrive/Documents/Unity Projects/Dreamwillow/Assets/Art/Characters/Player/PlayerHorizSprite.psb.meta";
	UnitySkeletonImporter::Import(tempPath, SkeletalAnimationLoader::Get()->skeleton, SkeletalAnimationLoader::Get()->skeletonDebugInfo);

	SkeletalAnimationTabSystem::Get()->AddTab(new MainView());
	SkeletalAnimationTabSystem::Get()->AddTab(new LayerHierarchyView());

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		SkeletalAnimationTabSystem::Get()->RenderTabs();

		renderingManager.EndFrame();
		ImGui::Render();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}