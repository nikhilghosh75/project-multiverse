#pragma once
#include <vulkan/vulkan.h>
#include <string>

enum TextureFilter : uint8_t
{
	Linear,
	Nearest,
	Cubic
};

class Texture
{
	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;
	VkSampler textureSampler;

public:
	Texture();
	Texture(const std::string& filepath);
	Texture(void* data, uint32_t _width, uint32_t _height);
	Texture(void* data, uint32_t _width, uint32_t _height, uint8_t _channels);

	~Texture();

	VkImageView GetImageView() inline const { return textureImageView; }
	VkSampler GetSampler() inline const { return textureSampler; }
	VkImage GetImage() inline const { return textureImage; }

	uint32_t GetTextureWidth() inline const { return textureWidth; }
	uint32_t GetTextureHeight() inline const { return textureHeight; }

	void TransitionLayout(VkImageLayout newLayout);

private:
	void SetupVulkanTexture(unsigned char* pixels);

	VkFormat GetIdealImageFormat(uint8_t _channels) const;

	uint32_t textureWidth;
	uint32_t textureHeight;

	VkImageLayout currentLayout;

	uint8_t textureChannels;
	bool isInitialized;
	TextureFilter textureFilter;
};