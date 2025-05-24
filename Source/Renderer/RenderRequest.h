#pragma once

#include "vulkan/vulkan.h"

class RenderRequest
{
public:
	virtual bool CanBeCombined(const RenderRequest* other) const = 0;

	virtual void CombineWith(RenderRequest* other) = 0;

	virtual void Render(VkCommandBuffer buffer) = 0;
	virtual void Clean() = 0;

	bool isActive = false; // Does the request have data that is actively used
	bool isProcessing = false; // Is the request being processed by the RenderingManager
};