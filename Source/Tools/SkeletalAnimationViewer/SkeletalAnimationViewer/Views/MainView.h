#pragma once

#include "../SkeletalAnimationTab.h"

#include "Device.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

class MainView : public SkeletalAnimationTab
{
public:
	MainView();

	void Render() override;

private:
	std::array<VkImage, MAX_FRAMES_IN_FLIGHT> offscreenImages;
	std::array<VkImageView, MAX_FRAMES_IN_FLIGHT> offscreenImageViews;
	std::array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> offscreenFramebuffers;
	std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> offscreenMemoryBuffers;
	VkSampler offscreenSampler;
	VkRenderPass offscreenRenderPass;
	std::array<ImTextureID, MAX_FRAMES_IN_FLIGHT> offscreenImguiTextures;

	int currentImageIndex = 0;
};