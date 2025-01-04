#include "ImGuiDevice.h"
#include "imgui.h"
#include "SwapChain.h"
#include "Window.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_win32.h"

void ImGuiDevice::Setup(HWND hwnd, Device* device)
{
    SetupDescriptorPool(device);
    SetupRenderPass(device);
    SetupCommandBuffer(device);
    SetupSyncObjects(device);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);

    QueueFamilyIndices indices = SwapChain::FindQueueFamilies(device->GetPhysicalDevice(), device->GetSurface());

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device->GetVulkanInstance();
    init_info.PhysicalDevice = device->GetPhysicalDevice();
    init_info.Device = device->GetVulkanDevice();
    init_info.QueueFamily = indices.graphicsFamily.value_or(-1);
    init_info.Queue = device->GetGraphicsQueue();
    init_info.RenderPass = imguiRenderPass;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;

    ImGui_ImplVulkan_Init(&init_info);

    Window::imguiEnabled = true;
}

void ImGuiDevice::StartFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDevice::EndFrame()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // We're recording commands for one-time use

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
    {
        // TODO: Handle Errors
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = imguiRenderPass;  // Use the render pass from the previous step
    renderPassInfo.framebuffer = Device::Get()->GetCurrentFramebuffer();  // The framebuffer, typically tied to the swapchain image
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Device::Get()->GetCurrentExtent();  // Swapchain extent (width/height)

    VkClearValue clearValue = {};
    clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };  // Clear color (optional)

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        // TODO: Handle Errors
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };  // Ensure the swapchain image is available before rendering
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };  // Wait at color attachment stage
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };  // Signal that rendering has finished
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {

    }
}

void ImGuiDevice::SetupDescriptorPool(Device* device)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200 },
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 200;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(device->GetVulkanDevice(), &pool_info, nullptr, &descriptorPool);
}

void ImGuiDevice::SetupRenderPass(Device* device)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = device->GetSwapChainFormat();  // Swapchain image format (e.g., VK_FORMAT_B8G8R8A8_UNORM)
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // No multi-sampling for ImGui
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;  // Load existing content (if you want to preserve content)
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // Store rendered content
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Initial layout before rendering
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // Layout for presentation

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;  // Attachment index in the render pass
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependency to transition the image from present to color attachment
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // External subpass doesn't require access flags
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device->GetVulkanDevice(), &renderPassInfo, nullptr, &imguiRenderPass) != VK_SUCCESS) 
    {
        // TODO: Handle Errors
    }
}

void ImGuiDevice::SetupCommandBuffer(Device* device)
{
    QueueFamilyIndices indices = SwapChain::FindQueueFamilies(device->GetPhysicalDevice(), device->GetSurface());

    VkCommandPool commandPool;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value_or(-1);  // The queue family that supports graphics commands
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // Allows command buffer reset

    if (vkCreateCommandPool(device->GetVulkanDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        // TODO: Handle Error
    }

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // Primary buffers can be submitted to a queue
    allocInfo.commandBufferCount = 1;  // Allocate a single buffer

    if (vkAllocateCommandBuffers(device->GetVulkanDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        // TODO: Handle Error
    }
}

void ImGuiDevice::SetupSyncObjects(Device* device)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device->GetVulkanDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS)
    {
        // TODO: Output the following error code
        // "Vulkan, failed to create semaphore
        exit(0);
    }
    if (vkCreateSemaphore(device->GetVulkanDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
    {
        // TODO: Output the following error code
        // "Vulkan, failed to create semaphore
        exit(0);
    }
}
