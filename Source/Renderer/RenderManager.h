#pragma once
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
private:
	FontRenderer* fontRenderer;
	ImageRenderer* imageRenderer;
	VectorRenderer* vectorRenderer;
};