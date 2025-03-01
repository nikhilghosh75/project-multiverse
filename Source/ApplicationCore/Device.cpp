#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"
#include "imgui.h"

#include <array>
#include <cstdlib>
#include <iostream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::array<const char*, 1> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::array<const char*, 4> extensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_EXT_debug_report", "VK_KHR_surface", "VK_KHR_win32_surface" };
const std::array<const char*, 1> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
const std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

Device::Device(HWND hwnd, HINSTANCE hinstance)
{
    device = this;
    instance = nullptr;

    // Several important subsystems 
    CreateInstance();
    SetupDebugMessenger();

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = hinstance;

    VULKAN_CALL_MSG(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface), "Failed to create a win32 surface");

    SetupPhysicalDevice();
    SetupLogicalDevice();

    swapChain.Setup(physicalDevice, surface);

    SetupCommandPool();
    SetupCommandBuffers();
    SetupSyncObjects();

    isSetupComplete = true;
}

void Device::StartFrame()
{
    shouldRenderFrame = true;
    DeviceFrame& frameObject = frameObjects[currentFrame % MAX_FRAMES_IN_FLIGHT];

    vkWaitForFences(vulkanDevice, 1, &frameObject.inFlightFence, VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(vulkanDevice, swapChain.GetSwapChain(), UINT64_MAX, frameObject.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        swapChain.Rebuild(physicalDevice, surface);
        shouldRenderFrame = false;
        return;
    }

    swapChain.PresentNextImage(currentImageIndex);

    VULKAN_CALL(vkResetFences(vulkanDevice, 1, &frameObject.inFlightFence));

    vkResetCommandBuffer(frameObject.commandBuffer, 0);
}

void Device::EndFrame()
{
    if (!shouldRenderFrame)
    {
        return;
    }

    DeviceFrame& frameObject = frameObjects[currentFrame % MAX_FRAMES_IN_FLIGHT];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { frameObject.imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = registeredCommandBuffers.size();
    submitInfo.pCommandBuffers = registeredCommandBuffers.data();

    VkSemaphore signalSemaphores[] = { frameObject.renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VULKAN_CALL_MSG(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameObject.inFlightFence), "Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain.GetSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &currentImageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || shouldResizeFramebuffer)
    {
        swapChain.Rebuild(physicalDevice, surface);
        shouldResizeFramebuffer = false;
    }
    else if (result != VK_SUCCESS)
    {
        ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "vkQueuePresentKHR(presentQueue, &presentInfo)", "Vulkan", result, "Failed to present the queue");
    }

    currentFrame++;
}

Device::~Device()
{
    vkDeviceWaitIdle(vulkanDevice);

    swapChain.Cleanup();

    for (int i = 0; i < frameObjects.size(); i++)
    {
        vkDestroySemaphore(vulkanDevice, frameObjects[i].imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(vulkanDevice, frameObjects[i].renderFinishedSemaphore, nullptr);
        vkDestroyFence(vulkanDevice, frameObjects[i].inFlightFence, nullptr);
    }
    vkDestroyCommandPool(vulkanDevice, commandPool, nullptr);
    vkDestroyDevice(vulkanDevice, nullptr);

    // Destroy Debug Utils
    // TO-DO: Disable in release mode
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, nullptr);
    }

    if (instance != nullptr)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    device = nullptr;
}

Device* Device::Get()
{
    return device;
}

VkDevice Device::GetVulkanDevice() const
{
    return vulkanDevice;
}

VkInstance Device::GetVulkanInstance() const
{
    return instance;
}

VkRenderPass& Device::GetRenderPass()
{
    return swapChain.GetRenderPass();
}

VkExtent2D Device::GetSwapChainExtent() const
{
    return swapChain.GetSwapExtent();
}

VkFormat Device::GetSwapChainFormat() const
{
    return swapChain.GetImageFormat();
}

VkFramebuffer Device::GetCurrentFramebuffer() const
{
    return swapChain.GetFramebuffer(currentImageIndex);
}

VkImage Device::GetCurrentSwapChainImage() const
{
    return swapChain.GetImage(currentImageIndex);
}

VkImageView Device::GetCurrentSwapChainImageView() const
{
    return swapChain.GetImageView(currentImageIndex);
}

VkCommandPool Device::GetCommandPool() const
{
    return commandPool;
}

VkCommandBuffer Device::GetCurrentCommandBuffer() const
{
    return frameObjects[currentFrame % MAX_FRAMES_IN_FLIGHT].commandBuffer;
}

VkQueue Device::GetGraphicsQueue() const
{
    return graphicsQueue;
}

VkPhysicalDevice Device::GetPhysicalDevice() const
{
    return physicalDevice;
}

VkPhysicalDeviceProperties Device::GetPhysicalDeviceProperties()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return properties;
}

VkSemaphore Device::GetCurrentImageAvailableSemaphore() const
{
    return frameObjects[currentFrame % MAX_FRAMES_IN_FLIGHT].imageAvailableSemaphore;
}

VkSurfaceKHR Device::GetSurface() const
{
    return surface;
}

size_t Device::GetFrameNumber()
{
    return currentFrame;
}

/// <summary>
/// Creates a buffer and allocates memory on
/// </summary>
/// <param name="size">The size of the buffer in bytes</param>
/// <param name="usage">How the buffer is used</param>
/// <param name="properties">The type of memory used</param>
/// <param name="buffer">A reference to the returned buffer</param>
/// <param name="bufferMemory">A reference to the returned memory</param>
void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VULKAN_CALL_MSG(vkCreateBuffer(vulkanDevice, &bufferInfo, nullptr, &buffer), "Failed to create buffer");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(vulkanDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

    VULKAN_CALL_MSG(vkAllocateMemory(vulkanDevice, &allocInfo, nullptr, &bufferMemory), "Cannot allocate buffer memory");

    vkBindBufferMemory(vulkanDevice, buffer, bufferMemory, 0);
}

/// <summary>
/// Allocates memory that is most efficient for GPu Access
/// </summary>
void Device::AllocateMemory(VkMemoryRequirements memoryRequirements, VkDeviceMemory& deviceMemory)
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VULKAN_CALL_MSG(vkAllocateMemory(vulkanDevice, &allocInfo, nullptr, &deviceMemory), "Cannot allocate buffer memory");
}

/// <summary>
/// Begins a command buffer
/// </summary>
VkCommandBuffer Device::BeginSingleTimeCommands()
{
    VkCommandBuffer commandBuffer = singleTimeCommandBuffers[currentSingleTimeCommandIndex];

    vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    currentSingleTimeCommandIndex = (currentSingleTimeCommandIndex + 1) % MAX_SINGLE_TIME_BUFFERS;

    return commandBuffer;
}

/// <summary>
/// Ends a command buffer by submitting the commands to the GPU. Does not free the command buffer
/// </summary>
void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    VULKAN_CALL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
}

/// <summary>
/// Copy a buffer
/// </summary>
void Device::CopyBuffer(VkBuffer sourceBuffer, VkBuffer destBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

/// <summary>
/// Put the image into the correct layout
/// </summary>
void Device::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

/// <summary>
/// Copy the entire image to the entire buffer
/// </summary>
void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(commandBuffer);
}

/// <summary>
/// Create's the device instance (which describes the API extensions enabled)
/// </summary>
void Device::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Project Multiverse";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Multiverse Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (!HasStandardValidationLayer())
    {
        ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "Device::HasStandardValidationLayer", "Device", 0, "Device does not have standard validation layers");
    }

    // TODO: Disable validation layers in release mode
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    VULKAN_CALL_MSG(vkCreateInstance(&createInfo, nullptr, &instance), "Cannot create instance");
}

/// <summary>
/// Sets up Validation Layers to ensure errors and issues get properly logged
/// </summary>
void Device::SetupDebugMessenger()
{
    // TODO: Disable Debug Messages in release builds
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, &createInfo, nullptr, &debugMessenger);
    }
}

/// <summary>
/// Find the first suitable GPU to use in rendering
/// </summary>
void Device::SetupPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "Device::SetupPhysicalDevice", "Device", 0, "Device Count is 0");
        exit(0);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto& device : devices)
    {
        if (IsSuitableDevice(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "Device::SetupPhysicalDevice", "Device", 0, "No suitable GPU found");
        exit(0);
    }
}

/// <summary>
/// Sets up the logical device, which describes the physical device features to use
/// </summary>
void Device::SetupLogicalDevice()
{
    QueueFamilyIndices indices = SwapChain::FindQueueFamilies(physicalDevice, surface);

    std::set<uint32_t> families = indices.GetAllUniqueFamilies();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (uint32_t family : families)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // TODO: Disable Validation Layers in release mode
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VULKAN_CALL_MSG(vkCreateDevice(physicalDevice, &createInfo, nullptr, &vulkanDevice), "No suitable logical device found");

    vkGetDeviceQueue(vulkanDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(vulkanDevice, indices.presentFamily.value(), 0, &presentQueue);
}

/// <summary>
/// Creates a command pool (an object to allocate command buffers)
/// </summary>
void Device::SetupCommandPool()
{
    QueueFamilyIndices indices = SwapChain::FindQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    VULKAN_CALL_MSG(vkCreateCommandPool(vulkanDevice, &poolInfo, nullptr, &commandPool), "Failed to create command pool");
}

void Device::SetupCommandBuffers()
{
    // Allocate one command buffer for each frame that can be in transit
    for (int i = 0; i < frameObjects.size(); i++)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VULKAN_CALL_MSG(vkAllocateCommandBuffers(vulkanDevice, &allocInfo, &frameObjects[i].commandBuffer), "Failed to create command buffers");
    }

    VkCommandBufferAllocateInfo singleTimeAllocInfo{};
    singleTimeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    singleTimeAllocInfo.commandPool = commandPool;
    singleTimeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    singleTimeAllocInfo.commandBufferCount = MAX_SINGLE_TIME_BUFFERS;

    VULKAN_CALL_MSG(vkAllocateCommandBuffers(vulkanDevice, &singleTimeAllocInfo, singleTimeCommandBuffers.data()), "Failed to create a single time command buffer");
}

void Device::SetupSyncObjects()
{
    for (int i = 0; i < frameObjects.size(); i++)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VULKAN_CALL_MSG(vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr, &frameObjects[i].imageAvailableSemaphore), "Failed to create semaphore");
        VULKAN_CALL_MSG(vkCreateSemaphore(vulkanDevice, &semaphoreInfo, nullptr, &frameObjects[i].renderFinishedSemaphore), "Failed to create semaphore");
        VULKAN_CALL_MSG(vkCreateFence(vulkanDevice, &fenceInfo, nullptr, &frameObjects[i].inFlightFence), "Failed to create fence");
    }
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
}

bool Device::HasStandardValidationLayer()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const VkLayerProperties& layer : availableLayers)
    {
        if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
        {
            return true;
        }
    }

    return false;
}

bool Device::IsSuitableDevice(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = SwapChain::FindQueueFamilies(device, surface);
    if (!indices.IsComplete())
        return false;

    if (!HasNecessaryDeviceExtensions(device))
        return false;

    if (!SupportsSwapChain(device))
        return false;

    return true;
}

bool Device::HasNecessaryDeviceExtensions(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Device::SupportsSwapChain(VkPhysicalDevice device)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount == 0)
        return false;

    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

    if (presentCount == 0)
        return false;

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
