#pragma once

#include "Rect.h"

#include "SkeletalSprite.h"
#include "Texture.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "imgui.h"

#include <map>
#include <optional>
#include <string>

ImVec2 GetTextureDimensions(Texture* texture);

class SkeletonDebugInfo
{
public:
	std::map<std::string, int> boneNameToIndex;
	int indicesSize = 0;
};

class LayerInfo
{
public:
	Texture* texture;
	VkDescriptorSet descriptorSet;
	float centerX;
	float centerY;
	float width;
	float height;
	Rect uvRect;

	std::vector<SpriteVertex> spriteVertices;
	std::vector<int> indices;
	void* originalLayer;
};

class SkeletalAnimationLoader
{
public:
	static SkeletalAnimationLoader* Get();

	void RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer, std::string path);
	void RegisterPhotoshopFile(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile);

	std::optional<std::string> FindFullNameOfLayer(const std::string& name);

	std::map<std::string, LayerInfo> layers;
	Texture* texture;
	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile;

	Skeleton skeleton;
	SkeletonDebugInfo skeletonDebugInfo;

private:
	void AddLayerToTexture(PhotoshopAPI::Layer<uint8_t>* layer, uint8_t* data, glm::vec2& currentPosition, float& currentRowHeight, float imageDimensions, Rect& uvRect);

	static SkeletalAnimationLoader* instance;
};