#include "SkeletalAnimationLoader.h"

#include "MathUtils.h"

#include "backends/imgui_impl_vulkan.h"

ImVec2 GetTextureDimensions(Texture* texture)
{
	return ImVec2(texture->GetTextureWidth(), texture->GetTextureHeight());
}

void CombineChannelData(uint8_t* redChannel, uint8_t* greenChannel, uint8_t* blueChannel, uint8_t* alphaChannel, uint8_t* data, uint32_t imageWidth, uint32_t imageHeight, uint32_t channelWidth, uint32_t channelHeight, glm::vec2 currentPosition)
{
	uint32_t imageSize = imageWidth * imageHeight * 4;

	for (uint32_t i = 0; i < channelWidth * channelHeight; i++)
	{
		uint32_t localx = i % channelWidth;
		uint32_t localy = i / channelWidth;
		uint32_t pos = (localx + (uint32_t)currentPosition.x) + (localy + (uint32_t)currentPosition.y) * imageWidth;

		data[pos * 4 + 0] = redChannel[i];
		data[pos * 4 + 1] = greenChannel[i];
		data[pos * 4 + 2] = blueChannel[i];
		data[pos * 4 + 3] = alphaChannel[i];
	}
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
		LayerInfo layerInfo;
		layerInfo.centerX = imageLayer->m_CenterX;
		layerInfo.centerY = imageLayer->m_CenterY;
		layerInfo.width = imageLayer->m_Width;
		layerInfo.height = imageLayer->m_Height;
		layerInfo.originalLayer = imageLayer;

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

	// Figure out the layers that are the largest
	struct LayerDimensionsInfo
	{
		std::string layerName;
		float area;
	};

	struct LayerDimensionsInfoCompator
	{
		bool operator() (LayerDimensionsInfo& a, LayerDimensionsInfo& b) { return a.area > b.area; }
	};

	std::vector<LayerDimensionsInfo> dimensions;
	float totalArea = 0;
	for (auto it : layers)
	{
		dimensions.push_back({ it.first, it.second.width * it.second.height });
		totalArea += it.second.width * it.second.height;
	}

	int imageDimensions = ceilf(sqrtf(totalArea * 2.40f) / 128.f) * 128.f;

	std::sort(dimensions.begin(), dimensions.end(), LayerDimensionsInfoCompator());

	uint8_t* data = (uint8_t*)malloc(imageDimensions * imageDimensions * 4);
	glm::vec2 currentImagePosition = glm::vec2(0, 0);
	float currentRowHeight = 0;

	for (int i = 0; i < dimensions.size(); i++)
	{
		PhotoshopAPI::Layer<uint8_t>* layer = (PhotoshopAPI::Layer<uint8_t>*)layers[dimensions[i].layerName].originalLayer;
		Rect& uvRect = layers[dimensions[i].layerName].uvRect;
		AddLayerToTexture(layer, data, currentImagePosition, currentRowHeight, imageDimensions, uvRect);
	}

	texture = new Texture(data, imageDimensions, imageDimensions);
	texture->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

std::optional<std::string> SkeletalAnimationLoader::FindFullNameOfLayer(const std::string& name)
{
	for (auto it : layers)
	{
		if (it.first.find(name) != std::string::npos)
		{
			return it.first;
		}
	}

	return std::nullopt;
}

void SkeletalAnimationLoader::AddLayerToTexture(PhotoshopAPI::Layer<uint8_t>* layer, uint8_t* data, glm::vec2& currentPosition, float& currentRowHeight, float imageDimensions, Rect& uvRect)
{
	PhotoshopAPI::ImageLayer<uint8_t>* imageLayer = dynamic_cast<PhotoshopAPI::ImageLayer<uint8_t>*>(layer);
	if (imageLayer)
	{
		// Split Image by Channel
		std::vector<uint8_t> redChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Red, true);
		std::vector<uint8_t> greenChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Green, true);
		std::vector<uint8_t> blueChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Blue, true);
		std::vector<uint8_t> alphaChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Alpha, true);

		if (currentPosition.x + imageLayer->m_Width + 2 >= layeredFile->m_Width)
		{
			currentPosition.x = 0;
			currentPosition.y = currentPosition.y + currentRowHeight;
			currentRowHeight = 0;
		}
		currentRowHeight = Math::Max(currentRowHeight, imageLayer->m_Height);

		CombineChannelData(redChannel.data(), greenChannel.data(), blueChannel.data(), alphaChannel.data(), data, imageDimensions, imageDimensions, layer->m_Width, layer->m_Height, currentPosition);
		
		uvRect.left = (float)currentPosition.x / (float)imageDimensions;
		uvRect.right = (float)(currentPosition.x + layer->m_Width) / (float)imageDimensions;
		uvRect.top = (float)currentPosition.y / (float)imageDimensions;
		uvRect.bottom = (float)(currentPosition.y + layer->m_Height) / (float)imageDimensions;

		currentPosition.x += imageLayer->m_Width + 2;
	}
}

