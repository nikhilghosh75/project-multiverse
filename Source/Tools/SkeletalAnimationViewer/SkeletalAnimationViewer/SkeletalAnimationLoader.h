#pragma once

#include "Texture.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "imgui.h"

#include <string>
#include <unordered_map>

ImVec2 GetTextureDimensions(Texture* texture);

class LayerInfo
{
public:
	Texture* texture;
	VkDescriptorSet descriptorSet;
	float centerX;
	float centerY;
	float width;
	float height;
};

class SkeletalAnimationLoader
{
public:
	static SkeletalAnimationLoader* Get();

	void RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer, std::string path);
	void RegisterPhotoshopFile(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile);

	std::unordered_map<std::string, LayerInfo> layers;
	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile;
private:
	static SkeletalAnimationLoader* instance;
};