#include "PhotoshopAPI/include/PhotoshopAPI.h"
#include "Device.h"
#include "Window.h"
#include "ImGuiDevice.h"
#include "RenderManager.h"

#include "imgui.h"

void RenderLayer(PhotoshopAPI::Layer<uint8_t>* layer)
{
	PhotoshopAPI::GroupLayer<uint8_t>* groupLayer = dynamic_cast<PhotoshopAPI::GroupLayer<uint8_t>*>(layer);

	if (groupLayer != nullptr)
	{
		bool nodeOpen = ImGui::TreeNodeEx(layer->m_LayerName.c_str());

		if (nodeOpen)
		{
			for (int i = 0; i < groupLayer->m_Layers.size(); i++)
			{
				RenderLayer(groupLayer->m_Layers[i].get());
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
		ImGui::TreeNodeEx(layer->m_LayerName.c_str(), flags);
		ImGui::TreePop();
	}
}

void RenderLayersWindow(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

	ImGui::Begin("Layers");
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RenderLayer(layeredFile->m_Layers[i].get());
	}
	ImGui::End();
}

int main()
{
	PhotoshopAPI::File inputFile = PhotoshopAPI::File("C:/Users/debgh/OneDrive/Documents/Unity Projects/Dreamwillow/Assets/Art/Characters/Player/PlayerHorizSprite.psb");

	std::unique_ptr<PhotoshopAPI::PhotoshopFile> file = std::make_unique<PhotoshopAPI::PhotoshopFile>();
	PhotoshopAPI::ProgressCallback callback;
	file->read(inputFile, callback);

	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile = new PhotoshopAPI::LayeredFile<uint8_t>(file);

	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;
	renderingManager.Setup();

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		RenderLayersWindow(layeredFile);

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}