#include "RenderManager.h"

#include "Device.h"

#include "DateTime.h"
#include "VulkanUtils.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "tracy/Tracy.hpp"

void MarkRenderRequestsAsProcessing(std::vector<RenderRequest*>& requestsLastFrame);
void CombineRenderRequests(std::vector<RenderRequest*>& requestsLastFrame);
void FreeRenderRequests(std::vector<RenderRequest*>& requestsLastFrame);
void BeginRenderCommandBuffer(VkCommandBuffer buffer);
void EndRenderCommandBuffer(VkCommandBuffer buffer);

void RunRenderThread(RenderManager* manager)
{
	manager->canStartRenderingFrame = false;
	while (manager->isRendererRunning)
	{
		// Wait until the game thread signals that we can start the frame
		while (!manager->canStartRenderingFrame)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

		ZoneScopedN("Render Loop");

		std::vector<RenderRequest*> requestsLastFrame = manager->GetRenderRequests();
		MarkRenderRequestsAsProcessing(requestsLastFrame);

		manager->StartFrame();
		manager->canStartRenderingFrame = false;
		manager->isFinishedRenderingFrame = false;

		if (Device::Get()->shouldRenderFrame)
		{
			int requestsBeforeCombining = requestsLastFrame.size();
			CombineRenderRequests(requestsLastFrame);
			int requestsAfterCombining = requestsLastFrame.size();

			std::cout << requestsBeforeCombining << " before, " << requestsAfterCombining << " after" << std::endl;

			VkCommandBuffer commandBuffer = manager->GetCurrentCommandBuffer();
			BeginRenderCommandBuffer(commandBuffer);

			for (int i = 0; i < requestsLastFrame.size(); i++)
			{
				requestsLastFrame[i]->Render(commandBuffer);
			}

			EndRenderCommandBuffer(commandBuffer);
			FreeRenderRequests(requestsLastFrame);
		}

		manager->EndFrame();
		manager->isFinishedRenderingFrame = true;
	}
}

void CombineRenderRequests(std::vector<RenderRequest*>& requestsLastFrame)
{
	ZoneScoped;
	if (requestsLastFrame.size() > 0)
	{
		std::vector<int> indicesToRemove;
		for (int i = 0; i < requestsLastFrame.size() - 1; i++)
		{
			if (requestsLastFrame[i]->CanBeCombined(requestsLastFrame[i + 1]))
			{
				requestsLastFrame[i + 1]->CombineWith(requestsLastFrame[i]);

				requestsLastFrame[i]->isActive = false;
				requestsLastFrame[i]->isProcessing = false;
				requestsLastFrame[i]->Clean();
				indicesToRemove.push_back(i);
			}
		}

		for (int i = indicesToRemove.size() - 1; i >= 0; i--)
		{
			requestsLastFrame.erase(requestsLastFrame.begin() + indicesToRemove[i]);
		}
	}
}

void FreeRenderRequests(std::vector<RenderRequest*>& requestsLastFrame)
{
	ZoneScoped;
	for (RenderRequest* request : requestsLastFrame)
	{
		request->isActive = false;
		request->isProcessing = false;
		request->Clean();
	}
}

void BeginRenderCommandBuffer(VkCommandBuffer buffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VULKAN_CALL(vkBeginCommandBuffer(buffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = Device::Get()->GetRenderPass();
	renderPassInfo.framebuffer = Device::Get()->GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = Device::Get()->GetCurrentExtent();

	VkClearValue clearColor = { {{1.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void EndRenderCommandBuffer(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);

	VULKAN_CALL_MSG(vkEndCommandBuffer(commandBuffer), "Failed to end command buffer");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());
}

void MarkRenderRequestsAsProcessing(std::vector<RenderRequest*>& requestsLastFrame)
{
	ZoneScoped;
	for (RenderRequest* request : requestsLastFrame)
	{
		request->isProcessing = true;
	}
}

RenderManager::RenderManager()
{
	debugRenderer = new DebugRenderer();
	fontRenderer = new FontRenderer();
	imageRenderer = new ImageRenderer();
	vectorRenderer = new VectorRenderer();

	// Create Commander Buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = Device::Get()->GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	VULKAN_CALL(vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, commandBuffers.data()));
}

RenderManager::~RenderManager()
{
	delete fontRenderer;
	delete imageRenderer;
	delete vectorRenderer;
	delete debugRenderer;
}

void RenderManager::StartFrame()
{
	ZoneScoped;

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

	currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void RenderManager::EndFrame()
{
	Device::Get()->TransitionImageLayout(
		Device::Get()->GetCurrentSwapChainImage(),
		Device::Get()->GetSwapChainFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

std::vector<RenderRequest*> RenderManager::GetRenderRequests()
{
	std::vector<RenderRequest*> requests;

	std::vector<RenderRequest*> imageRequests = ImageRenderRequest::GetRequestsThisFrame();
	for (RenderRequest* request : imageRequests)
	{
		requests.push_back(request);
	}

	std::vector<RenderRequest*> simpleVectorRequests = SimpleVectorRenderRequest::GetRequestsThisFrame();
	for (RenderRequest* request : simpleVectorRequests)
	{
		requests.push_back(request);
	}

	std::vector<RenderRequest*> fontRequests = FontRenderRequest::GetRequestsThisFrame();
	for (RenderRequest* request : fontRequests)
	{
		requests.push_back(request);
	}

	std::vector<RenderRequest*> debugRequests = DebugRenderRequest::GetRequestsThisFrame();
	for (RenderRequest* request : debugRequests)
	{
		requests.push_back(request);
	}

	return requests;
}

VkCommandBuffer RenderManager::GetCurrentCommandBuffer()
{
	return commandBuffers[currentIndex];
}

