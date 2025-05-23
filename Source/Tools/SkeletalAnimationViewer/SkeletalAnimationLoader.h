#pragma once

#include "SkeletalSpriteRenderer.h"
#include "Texture.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "imgui.h"

#include <map>
#include <optional>
#include <string>

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

	std::vector<SpriteVertex> spriteVertices;
};

class SkeletalAnimationLoader
{
public:
	static SkeletalAnimationLoader* Get();

	void RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer, std::string path);
	void RegisterPhotoshopFile(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile);

	std::optional<std::string> FindFullNameOfLayer(const std::string& name);

	std::map<std::string, LayerInfo> layers;
	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile;

	Skeleton skeleton;
private:
	static SkeletalAnimationLoader* instance;
};