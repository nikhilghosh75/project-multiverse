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

	void Setup();

	void StartFrame();
	void EndFrame();

	std::vector<RenderRequest*> GetRenderRequests();

	bool canStartRenderingFrame = true;
	bool isFinishedRenderingFrame = false;
	bool isRendererRunning = true;

private:
	DebugRenderer* debugRenderer;
	FontRenderer* fontRenderer;
	ImageRenderer* imageRenderer;
	VectorRenderer* vectorRenderer;
};

void RunRenderThread(RenderManager* manager);
