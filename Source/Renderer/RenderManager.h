#pragma once
#include "DebugRenderer.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "VectorRenderer.h"

class RenderManager
{
public:
	RenderManager();
	~RenderManager();

	void StartFrame();
	void EndFrame();

	std::vector<RenderRequest*> GetRenderRequests();

	VkCommandBuffer GetCurrentCommandBuffer();

	bool canStartRenderingFrame = true;
	bool isFinishedRenderingFrame = false;
	bool isRendererRunning = true;

private:
	DebugRenderer* debugRenderer;
	FontRenderer* fontRenderer;
	ImageRenderer* imageRenderer;
	VectorRenderer* vectorRenderer;

	unsigned int currentIndex = 0;
	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	std::array<VkCommandBuffer, MAX_REQUESTS_IN_FLIGHT> commandBuffers;
};
void RunRenderThread(RenderManager* manager);