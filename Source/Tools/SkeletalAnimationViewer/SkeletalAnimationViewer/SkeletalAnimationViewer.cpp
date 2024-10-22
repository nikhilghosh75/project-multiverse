#include "PhotoshopAPI/include/PhotoshopAPI.h"
#include "Device.h"
#include "Window.h"
#include "Texture.h"
#include "ImGuiDevice.h"
#include "RenderManager.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include <unordered_map>

ImVec2 GetTextureDimensions(Texture* texture)
{
	return ImVec2(texture->GetTextureWidth(), texture->GetTextureHeight());
}

uint8_t* CombineChannelData(uint8_t* redChannel, uint8_t* greenChannel, uint8_t* blueChannel, uint8_t* alphaChannel, uint32_t width, uint32_t height)
{
	uint8_t* data = (uint8_t*)malloc(width * height * 4);

	for (uint32_t i = 0; i < width * height; i++)
	{
		data[i * 4 + 0] = redChannel[i];
		data[i * 4 + 1] = greenChannel[i];
		data[i * 4 + 2] = blueChannel[i];
		data[i * 4 + 3] = alphaChannel[i];
	}

	return data;
}

class LayerInfo
{
public:
	Texture* texture;
	VkDescriptorSet descriptorSet;
};

std::unordered_map<std::string, LayerInfo> layers;
std::string currentLayer = "UNUSED";

void RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer)
{
	PhotoshopAPI::ImageLayer<uint8_t>* imageLayer = dynamic_cast<PhotoshopAPI::ImageLayer<uint8_t>*>(layer);
	PhotoshopAPI::GroupLayer<uint8_t>* groupLayer = dynamic_cast<PhotoshopAPI::GroupLayer<uint8_t>*>(layer);

	if (imageLayer != nullptr)
	{
		// Split Image by Channel
		std::vector<uint8_t> redChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Red, true);
		std::vector<uint8_t> greenChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Green, true);
		std::vector<uint8_t> blueChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Blue, true);
		std::vector<uint8_t> alphaChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Alpha, true);

		uint8_t* data = CombineChannelData(redChannel.data(), greenChannel.data(), blueChannel.data(), alphaChannel.data(), layer->m_Width, layer->m_Height);
		Texture* texture = new Texture(data, layer->m_Width, layer->m_Height);

		LayerInfo layerInfo;
		layerInfo.texture = texture;
		layerInfo.descriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), texture->GetImageLayout());

		layers.insert({ layer->m_LayerName, layerInfo });
	}
	else if (groupLayer != nullptr)
	{
		for (int i = 0; i < groupLayer->m_Layers.size(); i++)
		{
			RegisterLayer(groupLayer->m_Layers[i].get());
		}
	}
}

void SetupLayerInfos(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile)
{
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RegisterLayer(layeredFile->m_Layers[i].get());
	}
}

void RenderLayerHierarchy(PhotoshopAPI::Layer<uint8_t>* layer)
{
	PhotoshopAPI::GroupLayer<uint8_t>* groupLayer = dynamic_cast<PhotoshopAPI::GroupLayer<uint8_t>*>(layer);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

	if (layer->m_LayerName == currentLayer)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (groupLayer != nullptr)
	{
		bool nodeOpen = ImGui::TreeNodeEx(layer->m_LayerName.c_str(), flags);
		if (ImGui::IsItemClicked())
		{
			currentLayer = layer->m_LayerName;
		}

		if (nodeOpen)
		{
			for (int i = 0; i < groupLayer->m_Layers.size(); i++)
			{
				RenderLayerHierarchy(groupLayer->m_Layers[i].get());
			}
			ImGui::TreePop();
		}
	}
	else
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
		ImGui::TreeNodeEx(layer->m_LayerName.c_str(), flags);

		if (ImGui::IsItemClicked())
		{
			currentLayer = layer->m_LayerName;
		}

		ImGui::TreePop();
	}
}

void RenderLayersHierarchyWindow(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile)
{
	ImGui::Begin("Layers");
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RenderLayerHierarchy(layeredFile->m_Layers[i].get());
	}
	ImGui::End();
}

void RenderLayerImage()
{
	if (currentLayer == "UNUSED")
	{
		return;
	}

	auto foundLayer = layers.find(currentLayer);
	if (foundLayer == layers.end())
	{
		return;
	}

	LayerInfo& currentLayerInfo = (*foundLayer).second;

	ImGui::Begin("Layer Image");

	ImGui::Image((ImTextureID)currentLayerInfo.descriptorSet, GetTextureDimensions(currentLayerInfo.texture));

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

	SetupLayerInfos(layeredFile);

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		RenderLayersHierarchyWindow(layeredFile);
		RenderLayerImage();

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}