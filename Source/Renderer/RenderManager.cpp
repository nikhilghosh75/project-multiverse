#include "RenderManager.h"

#include "Device.h"

#include "DateTime.h"

#include <algorithm>
#include <iostream>
#include <thread>

#include "tracy/Tracy.hpp"

void MarkRenderRequestsAsProcessing(std::vector<RenderRequest*>& requestsLastFrame);
void SortRenderRequests(std::vector<RenderRequest*>& requestsLastFrame);
void CombineRenderRequests(std::vector<RenderRequest*>& requestsLastFrame);
void FreeRenderRequests(std::vector<RenderRequest*>& requestsLastFrame);

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

		// Don't let the game loop start adding more requests until we get all of the requests
		manager->StartFrame();
		manager->canStartRenderingFrame = false;
		manager->isFinishedRenderingFrame = false;

		if (Device::Get()->shouldRenderFrame)
		{
			int requestsBeforeCombining = requestsLastFrame.size();
			SortRenderRequests(requestsLastFrame);
			CombineRenderRequests(requestsLastFrame);
			int requestsAfterCombining = requestsLastFrame.size();

			// TODO: Make a better way to enable/disable debugging
			std::cout << requestsBeforeCombining << " before, " << requestsAfterCombining << " after" << std::endl;

			for (int i = 0; i < requestsLastFrame.size(); i++)
			{
				requestsLastFrame[i]->Render();
			}

			FreeRenderRequests(requestsLastFrame);
		}

		manager->EndFrame();
		manager->isFinishedRenderingFrame = true;
	}
}

void SortRenderRequests(std::vector<RenderRequest*>& requestsLastFrame)
{
	// For now, render requests are only sorted by order, and not by type
	ZoneScoped;
	std::sort(requestsLastFrame.begin(), requestsLastFrame.end(), [](RenderRequest* r1, RenderRequest* r2) { return r1->renderingOrder < r2->renderingOrder; });
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
	// Render Requests are not dynamically allocated (stored by the renderers), so they need to be set as inactive
	ZoneScoped;
	for (RenderRequest* request : requestsLastFrame)
	{
		request->isActive = false;
		request->isProcessing = false;
		request->Clean();
	}
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
	skeletalSpriteRenderer = new SkeletalSpriteRenderer();
}

RenderManager::~RenderManager()
{
	delete fontRenderer;
	delete imageRenderer;
	delete vectorRenderer;
	delete debugRenderer;
	delete skeletalSpriteRenderer;
}

void RenderManager::StartFrame()
{
	ZoneScoped;

	// Clear the swapchain image
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

std::vector<RenderRequest*> RenderManager::GetRenderRequests()
{
	// Since we only have a few renderers, it's easier to just call them manually
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

	std::vector<RenderRequest*> skeletalSpriteRequests = SkeletalSpriteRenderRequest::GetRequestsThisFrame();
	for (RenderRequest* request : skeletalSpriteRequests)
	{
		requests.push_back(request);
	}

	return requests;
}

