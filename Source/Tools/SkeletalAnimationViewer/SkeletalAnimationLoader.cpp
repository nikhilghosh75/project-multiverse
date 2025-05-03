#include "SkeletalAnimationLoader.h"

#include "backends/imgui_impl_vulkan.h"

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

SkeletalAnimationLoader* SkeletalAnimationLoader::instance = nullptr;

SkeletalAnimationLoader* SkeletalAnimationLoader::Get()
{
	if (instance == nullptr)
	{
		instance = new SkeletalAnimationLoader();
	}

	return instance;
}

void SkeletalAnimationLoader::RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer, std::string path)
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
		texture->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		LayerInfo layerInfo;
		layerInfo.texture = texture;
		layerInfo.descriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), texture->GetImageLayout());
		layerInfo.centerX = imageLayer->m_CenterX;
		layerInfo.centerY = imageLayer->m_CenterY;
		layerInfo.width = imageLayer->m_Width;
		layerInfo.height = imageLayer->m_Height;

		layers.insert({ path + layer->m_LayerName, layerInfo });
	}
	else if (groupLayer != nullptr)
	{
		for (int i = 0; i < groupLayer->m_Layers.size(); i++)
		{
			RegisterLayer(groupLayer->m_Layers[i].get(), path + layer->m_LayerName + "/");
		}
	}
}

void SkeletalAnimationLoader::RegisterPhotoshopFile(PhotoshopAPI::LayeredFile<uint8_t>* _layeredFile)
{
	layeredFile = _layeredFile;
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RegisterLayer(layeredFile->m_Layers[i].get(), "");
	}
}
