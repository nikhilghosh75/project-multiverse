#pragma once
#include <Windows.h>
#include "Device.h"

class ImGuiDevice
{
public:
	void Setup(HWND hwnd, Device* device);

	void StartFrame();
	void EndFrame();

private:
	void SetupDescriptorPool(Device* device);
	void SetupRenderPass(Device* device);
	void SetupCommandBuffer(Device* device);
	void SetupSyncObjects(Device* device);

	VkDescriptorPool descriptorPool;

	VkRenderPass imguiRenderPass;
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
};