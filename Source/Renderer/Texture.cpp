#include "Texture.h"

#include "Device.h"
#include "VulkanUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Texture::Texture()
{
	isInitialized = false;
}

Texture::Texture(const std::string& filepath)
{
	// Load the texture directly from a png file
	int _textureWidth, _textureHeight, _textureChannels;
	stbi_uc* pixels = stbi_load(filepath.c_str(), &_textureWidth, &_textureHeight, &_textureChannels, STBI_rgb_alpha);

	textureWidth = static_cast<uint32_t>(_textureWidth);
	textureHeight = static_cast<uint32_t>(_textureHeight);
	textureChannels = static_cast<uint8_t>(_textureChannels);

	SetupVulkanTexture(pixels);

	stbi_image_free(pixels);
	isInitialized = true;
}

Texture::Texture(void* data, uint32_t _width, uint32_t _height)
	: textureWidth(_width), textureHeight(_height), textureChannels(4)
{
	// Create the texture from the given data
	SetupVulkanTexture(reinterpret_cast<unsigned char*>(data));
	isInitialized = true;
}

Texture::Texture(void* data, uint32_t _width, uint32_t _height, uint8_t _channels)
	: textureWidth(_width), textureHeight(_height), textureChannels(_channels)
{
	SetupVulkanTexture(reinterpret_cast<unsigned char*>(data));
	isInitialized = true;
}

Texture::~Texture()
{
	vkDestroySampler(Device::Get()->GetVulkanDevice(), textureSampler, nullptr);
	vkDestroyImageView(Device::Get()->GetVulkanDevice(), textureImageView, nullptr);
	vkDestroyImage(Device::Get()->GetVulkanDevice(), textureImage, nullptr);
	vkFreeMemory(Device::Get()->GetVulkanDevice(), textureImageMemory, nullptr);
}

void Texture::TransitionLayout(VkImageLayout newLayout)
{
	if (currentLayout == newLayout)
	{
		return;
	}

	Device::Get()->TransitionImageLayout(textureImage, GetIdealImageFormat(textureChannels), currentLayout, newLayout);
	currentLayout = newLayout;
}

void Texture::SetupVulkanTexture(unsigned char* pixels)
{
	VkDeviceSize imageSize = textureHeight * textureWidth * textureChannels;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Device::Get()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	VULKAN_CALL(vkMapMemory(Device::Get()->GetVulkanDevice(), stagingBufferMemory, 0, imageSize, 0, &data));
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(Device::Get()->GetVulkanDevice(), stagingBufferMemory);

	VkFormat format = GetIdealImageFormat(textureChannels);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = textureWidth;
	imageInfo.extent.height = textureHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VULKAN_CALL_MSG(vkCreateImage(Device::Get()->GetVulkanDevice(), &imageInfo, nullptr, &textureImage), "Failed to create image");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Device::Get()->GetVulkanDevice(), textureImage, &memRequirements);

	Device::Get()->AllocateMemory(memRequirements, textureImageMemory);
	vkBindImageMemory(Device::Get()->GetVulkanDevice(), textureImage, textureImageMemory, 0);

	Device::Get()->TransitionImageLayout(textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Device::Get()->CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));

	currentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	vkDestroyBuffer(Device::Get()->GetVulkanDevice(), stagingBuffer, nullptr);
	vkFreeMemory(Device::Get()->GetVulkanDevice(), stagingBufferMemory, nullptr);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VULKAN_CALL_MSG(vkCreateImageView(Device::Get()->GetVulkanDevice(), &viewInfo, nullptr, &textureImageView), "Cannot create image view");

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = Device::Get()->GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VULKAN_CALL_MSG(vkCreateSampler(Device::Get()->GetVulkanDevice(), &samplerInfo, nullptr, &textureSampler), "Cannot create image sampler");

	textureFilter = TextureFilter::Linear;
}

VkFormat Texture::GetIdealImageFormat(uint8_t _channels) const
{
	if (_channels == 1)
	{
		return VK_FORMAT_R8_UNORM;
	}
	if (_channels == 4)
	{
		return VK_FORMAT_R8G8B8A8_SRGB;
	}

	return VK_FORMAT_R8G8B8A8_SRGB;
}
