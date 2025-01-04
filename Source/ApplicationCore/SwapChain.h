#pragma once
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	std::set<uint32_t> GetAllUniqueFamilies() const;

	inline bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct RenderPass
{
	VkRenderPass pass = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkExtent2D extents = { 0, 0 };
};

class SwapChain
{
public:
	void Setup(VkPhysicalDevice device, VkSurfaceKHR surface);

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	void Rebuild(VkPhysicalDevice device, VkSurfaceKHR surface);
	void Cleanup();

	void PresentNextImage(int currentImageIndex);

	VkFramebuffer GetFramebuffer(int currentImageIndex) const;
	VkImage GetImage(int currentImageIndex) const;
	VkImageView GetImageView(int currentImageIndex) const;
	VkFormat GetImageFormat() const;
	VkExtent2D GetSwapExtent() const;
	VkRenderPass& GetRenderPass();

	const VkSwapchainKHR& GetSwapChain() const;
private:

	VkSurfaceFormatKHR FindSwapChainFormat(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkPresentModeKHR FindSwapPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkExtent2D FindSwapExtent(VkPhysicalDevice device, VkSurfaceKHR surface);
	uint32_t FindSwapImageCount(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSwapchainKHR swapChain;
	VkRenderPass renderPass;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	QueueFamilyIndices indices;
};

