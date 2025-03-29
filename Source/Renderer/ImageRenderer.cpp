#include "ImageRenderer.h"

#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"

#include <array>

const std::vector<uint16_t> indices = 
{
    0, 1, 2, 2, 3, 0
};

std::array<ImageRenderRequest, ImageRenderRequest::MAX_IMAGE_REQUESTS> ImageRenderRequest::requests;

ImageRenderingOptions::ImageRenderingOptions()
{
    keepAspectRatio = false;
}

bool ImageRenderRequest::CanBeCombined(const RenderRequest* other) const
{
    return false;
}

void ImageRenderRequest::CombineWith(RenderRequest* other)
{
}

void ImageRenderRequest::Render()
{
    ImageRenderer::Get()->RenderImageRequest(this);
}

void ImageRenderRequest::Clean()
{
    texture = nullptr;
}

ImageRenderRequest* ImageRenderRequest::CreateRequest()
{
    if (!requestsArrayInitialized)
    {
        for (int i = 0; i < MAX_IMAGE_REQUESTS; i++)
        {
            requests[i].isActive = false;
        }
        requestsArrayInitialized = true;
    }

    lastIndex = (lastIndex + 1) % MAX_IMAGE_REQUESTS;
    requests[lastIndex].isActive = true;
    return &requests[lastIndex];
}

std::vector<RenderRequest*> ImageRenderRequest::GetRequestsThisFrame()
{
    std::vector<RenderRequest*> requestsThisFrame;

    for (ImageRenderRequest& request : requests)
    {
        if (request.isActive && !request.isProcessing)
        {
            requestsThisFrame.push_back(&request);
        }
    }

    return requestsThisFrame;
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
    if (options.keepAspectRatio)
        rect = FitRectToTexture(rect, texture);

    ImageRenderRequest* request = ImageRenderRequest::CreateRequest();
    request->texture = texture;
    request->rect = rect;

    ImageRenderingResult result;
    result.finalRect = rect;
    result.rendered = true;

    return result;
}

void ImageRenderer::RenderImageRequest(ImageRenderRequest* request)
{
    SetTexture(request->texture);
    PopulateWithRect(request->rect);

    UpdateDescriptorSets();
    PopulateBuffers();
    DispatchCommands();

    vertices.clear();

    currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void ImageRenderer::SetTexture(Texture* texture)
{
    currentTexture = texture;
}

void ImageRenderer::PopulateWithRect(Rect rect)
{
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
}

Rect ImageRenderer::FitRectToTexture(Rect currentRect, Texture* texture)
{
    if (!texture)
        return currentRect;

    int width, height;
    Window::GetWindowSize(&width, &height);

    glm::vec2 screenDimensions = glm::vec2(
        (currentRect.right - currentRect.left) * (float)width, (currentRect.bottom - currentRect.top) * (float)height);
    float currentRatio = (float)screenDimensions.x / (float)screenDimensions.y;
    float imageRatio = (float)texture->GetTextureWidth() / (float)texture->GetTextureHeight();

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

    VULKAN_CALL(vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, commandBuffers.data()));
}

void ImageRenderer::CreatePipeline()
{
    Shader vertexShader = Shader("Data/Shaders/texture_vert.spv", Device::Get()->GetVulkanDevice());
    Shader fragmentShader = Shader("Data/Shaders/texture_frag.spv", Device::Get()->GetVulkanDevice());

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

    VULKAN_CALL(vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout));
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

    VULKAN_CALL(vkCreateDescriptorPool(Device::Get()->GetVulkanDevice(), &poolInfo, nullptr, &descriptorPool));
}

void ImageRenderer::CreateDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VULKAN_CALL(vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, &descriptorSet));
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

    VULKAN_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

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

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    VULKAN_CALL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());
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