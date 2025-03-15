#include "RenderManager.h"
#include "Device.h"

void RunRenderThread(RenderManager& manager)
{
	while (manager.isRendererRunning)
	{
		if (manager.canStartFrame)
		{
			manager.StartFrame();
			manager.canStartFrame = false;
		}

		manager.EndFrame();

	}
}

RenderManager::RenderManager()
{

}

RenderManager::~RenderManager()
{
	delete fontRenderer;
	delete imageRenderer;
	delete vectorRenderer;
	delete debugRenderer;
}

void RenderManager::Setup()
{	
	debugRenderer = new DebugRenderer();
	fontRenderer = new FontRenderer();
	imageRenderer = new ImageRenderer();
	vectorRenderer = new VectorRenderer();
}

void RenderManager::RunRenderThread()
{

}

void RenderManager::StartFrame()
{
	Device::Get()->TransitionImageLayout(
		Device::Get()->GetCurrentSwapChainImage(),
		Device::Get()->GetSwapChainFormat(),
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageSubresourceRange imageSubresourceRange;
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	VkCommandBuffer commandBuffer = Device::Get()->BeginSingleTimeCommands();

	VkClearColorValue clearColorValue = { 0.0, 0.0, 0.0, 0.0 };
	vkCmdClearColorImage(commandBuffer, Device::Get()->GetCurrentSwapChainImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &imageSubresourceRange);

	Device::Get()->EndSingleTimeCommands(commandBuffer);

	Device::Get()->TransitionImageLayout(
		Device::Get()->GetCurrentSwapChainImage(),
		Device::Get()->GetSwapChainFormat(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void RenderManager::EndFrame()
{
	Device::Get()->TransitionImageLayout(
		Device::Get()->GetCurrentSwapChainImage(),
		Device::Get()->GetSwapChainFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

