#include "ImageRenderer.h"
#include "Device.h"
#include "Window.h"
#include <array>

const std::vector<uint16_t> indices = 
{
    0, 1, 2, 2, 3, 0
};

ImageRenderingOptions::ImageRenderingOptions()
{
    keepAspectRatio = false;
}

ImageRenderer::ImageRenderer()
{
    instance = this;

    CreateBuffers();
    CreateDescriptorSetLayout();
	CreatePipeline();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

ImageRenderer::~ImageRenderer()
{
    vkFreeCommandBuffers(Device::Get()->GetVulkanDevice(), Device::Get()->GetCommandPool(), commandBuffers.size(), commandBuffers.data());
}

ImageRenderer* ImageRenderer::Get()
{
    return instance;
}

ImageRenderingResult ImageRenderer::AddImage(Texture* texture, Rect rect, ImageRenderingOptions options)
{
    currentTexture = texture;

    if (options.keepAspectRatio)
        rect = FitRectToTexture(rect);

    glm::vec2 bottomLeft = glm::vec2(rect.left, rect.bottom);
    glm::vec2 topLeft = glm::vec2(rect.left, rect.top);
    glm::vec2 topRight = glm::vec2(rect.right, rect.top);
    glm::vec2 bottomRight = glm::vec2(rect.right, rect.bottom);

    Vertex bottomLeftVertex(topRight, glm::vec2(0.0f, 0.0f));
    Vertex topLeftVertex(bottomRight, glm::vec2(0.0f, 1.0f));
    Vertex topRightVertex(bottomLeft, glm::vec2(1.0f, 1.0f));
    Vertex bottomRightVertex(topLeft, glm::vec2(1.0f, 0.0f));

    vertices.push_back(bottomLeftVertex);
    vertices.push_back(topLeftVertex);
    vertices.push_back(topRightVertex);
    vertices.push_back(bottomRightVertex);

    Render();

    ImageRenderingResult result;
    result.finalRect = rect;
    result.rendered = true;

    return result;
}

void ImageRenderer::Render()
{
    UpdateDescriptorSets();
    PopulateBuffers();
    DispatchCommands();
}

Rect ImageRenderer::FitRectToTexture(Rect currentRect)
{
    if (!currentTexture)
        return currentRect;

    int width, height;
    Window::GetWindowSize(&width, &height);

    glm::vec2 screenDimensions = glm::vec2(
        (currentRect.right - currentRect.left) * (float)width, (currentRect.bottom - currentRect.top) * (float)height);
    float currentRatio = (float)screenDimensions.x / (float)screenDimensions.y;
    float imageRatio = (float)currentTexture->GetTextureWidth() / (float)currentTexture->GetTextureHeight();

    if (currentRatio > imageRatio)
    {
        screenDimensions.x = screenDimensions.y * imageRatio;
    }
    else if (currentRatio < imageRatio)
    {
        screenDimensions.y = screenDimensions.x / imageRatio;
    }

    glm::vec2 windowDimensions = glm::vec2(screenDimensions.x / width, screenDimensions.y / height);
    currentRect.ResizeFromCenter(windowDimensions.x, windowDimensions.y);

    return currentRect;
}

void ImageRenderer::CreateBuffers()
{
    for (int i = 0; i < MAX_REQUESTS_IN_FLIGHT; i++)
    {
        VkDeviceSize vertexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(Vertex);
        Device::Get()->CreateBuffer(vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBuffers[i],
            vertexBufferMemories[i]);

        VkDeviceSize indexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(unsigned int);
        Device::Get()->CreateBuffer(indexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBuffers[i],
            indexBufferMemories[i]);
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = Device::Get()->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        exit(0);
    }
}

void ImageRenderer::CreatePipeline()
{
    Shader vertexShader = Shader::ReadShader("Data/Shaders/texture_vert.spv", Device::Get()->GetVulkanDevice());
    Shader fragmentShader = Shader::ReadShader("Data/Shaders/texture_frag.spv", Device::Get()->GetVulkanDevice());

    pipeline.SetShader(vertexShader, ShaderType::Vertex);
    pipeline.SetShader(fragmentShader, ShaderType::Fragment);

    pipeline.SetDescriptorSet(descriptorSetLayout);

    pipeline.SetAttributes(Vertex::GetAttributeDescriptions());
    pipeline.SetBinding(Vertex::GetBindingDescription());
    pipeline.SetColorBlendingEnabled(true);

    pipeline.Create();
}

void ImageRenderer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
    {
        exit(0);
    }
}

void ImageRenderer::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(Device::Get()->GetVulkanDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
    {
        exit(0);
    }
}

void ImageRenderer::CreateDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkResult result = vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        // TODO: Vulkan Error
        exit(0);
    }
}

void ImageRenderer::UpdateDescriptorSets()
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = currentTexture->GetImageView();
    imageInfo.sampler = currentTexture->GetSampler();

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(Device::Get()->GetVulkanDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    currentTexture->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void ImageRenderer::PopulateBuffers()
{
    Device::Get()->PopulateBufferFromVector(vertices, vertexBuffers[currentIndex], vertexBufferMemories[currentIndex]);
    Device::Get()->PopulateBufferFromVector(indices, indexBuffers[currentIndex], indexBufferMemories[currentIndex]);
}

void ImageRenderer::DispatchCommands()
{
    VkCommandBuffer commandBuffer = commandBuffers[currentIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        // TODO: Error Code
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = Device::Get()->GetRenderPass();
    renderPassInfo.framebuffer = Device::Get()->GetCurrentFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Device::Get()->GetSwapChainExtent();

    VkClearValue clearColor = { {{1.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(Device::Get()->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(Device::Get()->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = Device::Get()->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer bindableVertexBuffers[] = { vertexBuffers[currentIndex]};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, bindableVertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT16);

    // VkDescriptorSet& descriptorSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        // TODO: Output the following error code
        // "Vulkan, failed to end command buffer
        exit(0);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());

    vertices.clear();

    currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

ImageRenderer::Vertex::Vertex(glm::vec2 _position, glm::vec2 _uv)
	: position(_position), uv(_uv)
{
}

VkVertexInputBindingDescription ImageRenderer::Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> ImageRenderer::Vertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    VkVertexInputAttributeDescription positionDescription;
    positionDescription.binding = 0;
    positionDescription.location = 0;
    positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
    positionDescription.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription uvCoordinateDescription;
    uvCoordinateDescription.binding = 0;
    uvCoordinateDescription.location = 1;
    uvCoordinateDescription.format = VK_FORMAT_R32G32_SFLOAT;
    uvCoordinateDescription.offset = offsetof(Vertex, uv);

    attributeDescriptions.push_back(positionDescription);
    attributeDescriptions.push_back(uvCoordinateDescription);

    return attributeDescriptions;
}
