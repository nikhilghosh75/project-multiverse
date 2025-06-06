#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "SwapChain.h"
#include <vulkan/vulkan.h>
#include <array>
#include <optional>
#include <set>
#include <vector>
#include <windows.h>

// A struct containing all the resources needed for a single frame
// Useful to start construction of one frame while another finishes rendering
struct DeviceFrame
{
	VkCommandBuffer commandBuffer;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
};

// A struct containing all the data about staging buffers
// A staging buffer is a buffer used to stage data between the CPU and GPU
struct StagingBuffer
{
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};

const int MAX_FRAMES_IN_FLIGHT = 2;

// A class representing the Vulkan device
class Device
{
public:
	Device(HWND hwnd, HINSTANCE hinstance);

	void StartFrame();
	void EndFrame();

	// Should the current set of framebuffers be scrapped next frame for resizing
	bool shouldResizeFramebuffer = false;

	bool shouldRenderFrame = true;

	~Device();

	static Device* Get();

	VkDevice GetVulkanDevice() const;
	VkInstance GetVulkanInstance() const;
	VkRenderPass& GetRenderPass();
	VkExtent2D GetCurrentExtent() const;
	VkFormat GetSwapChainFormat() const;
	VkFramebuffer GetCurrentFramebuffer() const;
	VkImage GetCurrentSwapChainImage() const;
	VkImageView GetCurrentSwapChainImageView() const;
	VkCommandPool GetCommandPool() const;
	VkCommandBuffer GetCurrentCommandBuffer() const;
	VkQueue GetGraphicsQueue() const;
	VkPhysicalDevice GetPhysicalDevice() const;
	VkPhysicalDeviceProperties GetPhysicalDeviceProperties();
	VkSemaphore GetCurrentImageAvailableSemaphore() const;
	VkSurfaceKHR GetSurface() const;

	static size_t GetFrameNumber();

	// Create a Vulkan Buffer from the device (usually a vertex or index buffer
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
	// Allocate memory on the GPU
	void AllocateMemory(VkMemoryRequirements memoryRequirements, VkDeviceMemory& deviceMemory);

	// Create a command buffer for a single command
	VkCommandBuffer BeginSingleTimeCommands();

	// Submit a command buffer for a single command
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Copy the contents of one buffer to another buffer
	void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size);

	// Transition an image from one layout to another
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	// Copy of the contents of a buffer into an image (using the buffer as pixel data)
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	// Sets the override render (i.e. a renderpass not in the swap chain, and thus not rendered)
	void SetOverrideRenderPass(RenderPass pass);
	void ClearOverrideRenderPass();

	// Create a buffer from a vector (uploading the contents of the vector into the buffer's memory)
	template<typename T> void CreateBufferFromVector(const std::vector<T>& vector, VkBuffer& buffer, VkDeviceMemory& memory, VkBufferUsageFlags flags);

	template<typename T> void PopulateBufferFromVector(const std::vector<T>& vector, VkBuffer& buffer, VkDeviceMemory& memory, VkCommandBuffer commandBuffer);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	static inline Device* device;

	void CreateInstance();
	void SetupDebugMessenger();
	void SetupPhysicalDevice();
	void SetupLogicalDevice();
	void SetupCommandPool();
	void SetupCommandBuffers();
	void SetupSyncObjects();
	void SetupStagingBuffers();

	bool HasStandardValidationLayer();
	bool IsSuitableDevice(VkPhysicalDevice device);
	bool HasNecessaryDeviceExtensions(VkPhysicalDevice device);
	bool SupportsSwapChain(VkPhysicalDevice device);

	static const int MAX_SINGLE_TIME_BUFFERS = 20;
	static const int MAX_STAGING_BUFFERS = 10;

	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice vulkanDevice;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkCommandPool commandPool;

	std::optional<RenderPass> overrideRenderpass;

	SwapChain swapChain;

	std::vector<VkCommandBuffer> registeredCommandBuffers;

	std::array<DeviceFrame, MAX_FRAMES_IN_FLIGHT> frameObjects;
	std::array<VkCommandBuffer, MAX_SINGLE_TIME_BUFFERS> singleTimeCommandBuffers;
	std::array<StagingBuffer, MAX_STAGING_BUFFERS> stagingBuffers;

	uint32_t currentImageIndex;
	uint32_t currentSingleTimeCommandIndex = 0;
	uint32_t currentStagingBufferIndex = 0;

	VkDebugUtilsMessengerEXT debugMessenger;

	bool isSetupComplete = false;

	static inline size_t currentFrame = 0;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

template<typename T>
inline void Device::CreateBufferFromVector(const std::vector<T>& vector, VkBuffer& buffer, VkDeviceMemory& memory, VkBufferUsageFlags flags)
{
	VkDeviceSize bufferSize = sizeof(vector[0]) * vector.size();

	VkBuffer stagingBuffer = stagingBuffers[currentStagingBufferIndex].buffer;
	VkDeviceMemory stagingBufferMemory = stagingBuffers[currentStagingBufferIndex].bufferMemory;

	void* data;
	vkMapMemory(vulkanDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vector.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

	CopyBuffer(stagingBuffer, buffer, bufferSize);

	currentStagingBufferIndex = (currentStagingBufferIndex + 1) % MAX_STAGING_BUFFERS;
}

template<typename T>
inline void Device::PopulateBufferFromVector(const std::vector<T>& vector, VkBuffer& buffer, VkDeviceMemory& memory, VkCommandBuffer commandBuffer)
{
	VkDeviceSize bufferSize = sizeof(vector[0]) * vector.size();

	VkBuffer stagingBuffer = stagingBuffers[currentStagingBufferIndex].buffer;
	VkDeviceMemory stagingBufferMemory = stagingBuffers[currentStagingBufferIndex].bufferMemory;

	void* data;
	vkMapMemory(vulkanDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vector.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanDevice, stagingBufferMemory);

	VkBufferCopy copyRegion{};
	copyRegion.size = bufferSize;
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);

	currentStagingBufferIndex = (currentStagingBufferIndex + 1) % MAX_STAGING_BUFFERS;
}