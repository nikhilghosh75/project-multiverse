#include "SwapChain.h"
#include "Device.h"
#include "Window.h"

std::set<uint32_t> QueueFamilyIndices::GetAllUniqueFamilies() const
{
    std::set<uint32_t> families;

    if (graphicsFamily)
        families.insert(graphicsFamily.value());

    if (presentFamily)
        families.insert(presentFamily.value());

    return families;
}


/// <summary>
/// Sets up the mechanism to swap images and present them to the green
/// </summary>
void SwapChain::Setup(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR surfaceFormat = FindSwapChainFormat(device, surface);
    VkPresentModeKHR presentMode = FindSwapPresentMode(device, surface);
    VkExtent2D extent = FindSwapExtent(device, surface);
    uint32_t imageCount = FindSwapImageCount(device, surface);

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageExtent = extent;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    indices = FindQueueFamilies(device, surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(Device::Get()->GetVulkanDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        // TODO: Output the following error code
        // "Vulkan, failed to create swap chain"
        exit(0);
    }

    // Specify that the render pass has only a single color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Device::Get()->GetVulkanDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        // TODO: Output the following error code
        // "Vulkan, failed to create render pass"
        exit(0);
    }

    vkGetSwapchainImagesKHR(Device::Get()->GetVulkanDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(Device::Get()->GetVulkanDevice(), swapChain, &imageCount, swapChainImages.data());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(Device::Get()->GetVulkanDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            // TODO: Output the following error code
            // "Vulkan, failed to create image view"
            exit(0);
        }
    }

    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
        swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = Device::Get()->GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Device::Get()->GetVulkanDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            // TODO: Output the following error code
            // "Vulkan, failed to create framebuffer"
            exit(0);
        }
    }
}

QueueFamilyIndices SwapChain::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices queueIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilies.size(); i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            queueIndices.presentFamily = i;
        }

        if (queueIndices.IsComplete())
        {
            break;
        }
    }

    return queueIndices;
}

void SwapChain::Rebuild(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vkDeviceWaitIdle(Device::Get()->GetVulkanDevice());
    Cleanup();
    Setup(device, surface);
}

void SwapChain::Cleanup()
{
    for (int i = 0; i < swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(Device::Get()->GetVulkanDevice(), swapChainFramebuffers[i], nullptr);
    }

    for (int i = 0; i < swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(Device::Get()->GetVulkanDevice(), swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(Device::Get()->GetVulkanDevice(), swapChain, nullptr);
}

void SwapChain::PresentNextImage(int currentImageIndex)
{
    Device::Get()->TransitionImageLayout(swapChainImages[currentImageIndex], swapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

VkFramebuffer SwapChain::GetFramebuffer(int currentImageIndex) const
{
    return swapChainFramebuffers[currentImageIndex];
}

VkImage SwapChain::GetImage(int currentImageIndex) const
{
    return swapChainImages[currentImageIndex];
}

VkImageView SwapChain::GetImageView(int currentImageIndex) const
{
    return swapChainImageViews[currentImageIndex];
}

VkFormat SwapChain::GetImageFormat() const
{
    return swapChainImageFormat;
}

VkExtent2D SwapChain::GetSwapExtent() const
{
    return swapChainExtent;
}

VkRenderPass& SwapChain::GetRenderPass()
{
    return renderPass;
}

const VkSwapchainKHR& SwapChain::GetSwapChain() const
{
    return swapChain;
}

VkSurfaceFormatKHR SwapChain::FindSwapChainFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());

    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR SwapChain::FindSwapPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

    std::vector<VkPresentModeKHR> modes(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, modes.data());

    for (const VkPresentModeKHR& mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::FindSwapExtent(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    if (capabilities.currentExtent.width < 100000)
    {
        return capabilities.currentExtent;
    }

    int width;
    int height;
    Window::GetWindowSize(&width, &height);

    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    return extent;
}

uint32_t SwapChain::FindSwapImageCount(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    return imageCount;
}
