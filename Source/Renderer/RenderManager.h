#pragma once
#include "DebugRenderer.h"
#include "FontRenderer.h"
#include "ImageRenderer.h"
#include "SkeletalSpriteRenderer.h"
#include "VectorRenderer.h"

/*
* It is expected at every project that renders to the screen creates a RenderManager
*/
class RenderManager
{
public:
	RenderManager();
	~RenderManager();

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
	SkeletalSpriteRenderer* skeletalSpriteRenderer;
	VectorRenderer* vectorRenderer;
};

void RunRenderThread(RenderManager* manager);
