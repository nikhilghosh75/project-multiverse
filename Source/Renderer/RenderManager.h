#pragma once
#include "DebugRenderer.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "VectorRenderer.h"

void RunRenderThread(RenderManager& manager);

class RenderManager
{
public:
	RenderManager();
	~RenderManager();

	void Setup();

	void StartFrame();
	void EndFrame();

	bool canStartFrame;
	bool isRendererRunning = true;
private:
	void ProcessRequestsFromLastFrame();

	DebugRenderer* debugRenderer;
	FontRenderer* fontRenderer;
	ImageRenderer* imageRenderer;
	VectorRenderer* vectorRenderer;
};