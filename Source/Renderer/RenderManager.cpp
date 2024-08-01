#include "RenderManager.h"
#include "Device.h"

RenderManager::RenderManager()
{

}

RenderManager::~RenderManager()
{
	delete fontRenderer;
	delete imageRenderer;
}

void RenderManager::Setup()
{	
	fontRenderer = new FontRenderer();
	imageRenderer = new ImageRenderer();
}

void RenderManager::StartFrame()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = Device::Get()->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		// TODO: Error Code
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = Device::Get()->GetRenderPass();
	renderPassInfo.framebuffer = Device::Get()->GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = Device::Get()->GetSwapChainExtent();

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 0.0f}} };
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkClearAttachment colorAttachment = {};
	colorAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorAttachment.colorAttachment = 0;
	colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

	// Specify the clear rectangle
	VkClearRect clearRect = {};
	clearRect.rect.offset = { 0, 0 };
	clearRect.rect.extent = Device::Get()->GetSwapChainExtent();
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	// Record the clear command
	vkCmdClearAttachments(commandBuffer, 1, &colorAttachment, 1, &clearRect);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		// TODO: Output the following error code
		// "Vulkan, failed to end command buffer
		exit(0);
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());
}

void RenderManager::EndFrame()
{
	// See if any requests can be combined
}
