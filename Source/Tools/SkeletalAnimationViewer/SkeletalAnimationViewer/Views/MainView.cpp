#include "MainView.h"

#include "../SkeletalAnimationLoader.h"

#include "Rect.h"

#include "ImageRenderer.h"

MainView::MainView()
	: SkeletalAnimationTab("Main View", false)
{
	unsigned int width = 5000;
	unsigned int height = 5000;

	for (int i = 0; i < offscreenImages.size(); i++)
	{
		// Create image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
		imageInfo.extent = { width, height, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkCreateImage(Device::Get()->GetVulkanDevice(), &imageInfo, nullptr, &offscreenImages[i]);

		// Allocate and bind memory
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(Device::Get()->GetVulkanDevice(), offscreenImages[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Device::Get()->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkAllocateMemory(Device::Get()->GetVulkanDevice(), &allocInfo, nullptr, &offscreenMemoryBuffers[i]);
		vkBindImageMemory(Device::Get()->GetVulkanDevice(), offscreenImages[i], offscreenMemoryBuffers[i], 0);
	}

	for (int i = 0; i < offscreenImageViews.size(); i++)
	{
		// Create image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = offscreenImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(Device::Get()->GetVulkanDevice(), &viewInfo, nullptr, &offscreenImageViews[i]);
	}

	// Create Render Pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = Device::Get()->GetSwapChainFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(Device::Get()->GetVulkanDevice(), &renderPassInfo, nullptr, &offscreenRenderPass) != VK_SUCCESS)
	{
		// TODO: Output the following error code
		// "Vulkan, failed to create render pass"
		exit(0);
	}


	// Create framebuffer
	for (int i = 0; i < offscreenFramebuffers.size(); i++)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = offscreenRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &offscreenImageViews[i];
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		vkCreateFramebuffer(Device::Get()->GetVulkanDevice(), &framebufferInfo, nullptr, &offscreenFramebuffers[i]);
	}

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; // Use linear filtering for magnification
	samplerInfo.minFilter = VK_FILTER_LINEAR; // Use linear filtering for minification
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Clamp texture coordinates
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE; // Enable anisotropic filtering if supported
	samplerInfo.maxAnisotropy = 16.0f; // Adjust as needed; check device limits
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color for out-of-range coordinates
	samplerInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized texture coordinates (0.0 to 1.0)
	samplerInfo.compareEnable = VK_FALSE; // Disable comparison for shadow maps
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Use linear interpolation for mipmaps
	samplerInfo.mipLodBias = 0.0f; // No bias
	samplerInfo.minLod = 0.0f; // Minimum level of detail
	samplerInfo.maxLod = 0.0f; // Maximum level of detail (set to 0 for no mipmapping)

	vkCreateSampler(Device::Get()->GetVulkanDevice(), &samplerInfo, nullptr, &offscreenSampler);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		Device::Get()->TransitionImageLayout(offscreenImages[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		offscreenImguiTextures[i] = ImGui_ImplVulkan_AddTexture(offscreenSampler, offscreenImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void MainView::Render()
{
	currentImageIndex = (currentImageIndex + 1) % MAX_FRAMES_IN_FLIGHT;

	Device::Get()->TransitionImageLayout(offscreenImages[currentImageIndex], VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	int halfWidth = 2500;
	int halfHeight = 2500;
	unsigned int width = 5000;
	unsigned int height = 5000;

	RenderPass renderPass;
	renderPass.extents = { width, height };
	renderPass.framebuffer = offscreenFramebuffers[currentImageIndex];
	renderPass.pass = offscreenRenderPass;

	Device::Get()->SetOverrideRenderPass(renderPass);

	// Render all of the images in the file one at a time
	for (auto& layer : SkeletalAnimationLoader::Get()->layers)
	{
		Rect rect(halfHeight + layer.second.centerY, halfHeight + layer.second.centerY + layer.second.height,
			halfWidth + layer.second.centerX, halfWidth + layer.second.centerX + layer.second.width);

		rect.top /= height;
		rect.bottom /= height;
		rect.left /= width;
		rect.right /= width;

		ImageRenderer::Get()->AddImage(layer.second.texture, rect);
	}

	std::vector<RenderRequest*> imageRenderRequests = ImageRenderRequest::GetRequestsThisFrame();
	for (int i = 0; i < imageRenderRequests.size(); i++)
	{
		imageRenderRequests[i]->Render();
		imageRenderRequests[i]->isActive = false;
		imageRenderRequests[i]->isProcessing = false;
		imageRenderRequests[i]->Clean();
	}

	Device::Get()->ClearOverrideRenderPass();

	Device::Get()->TransitionImageLayout(offscreenImages[currentImageIndex], VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ImGui::Image(offscreenImguiTextures[currentImageIndex], ImVec2(400, 400));
}
