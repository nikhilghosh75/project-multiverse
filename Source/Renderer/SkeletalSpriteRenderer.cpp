#include "SkeletalSpriteRenderer.h"

#include "Shader.h"

#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"

#include "tracy/Tracy.hpp"

std::array<SkeletalSpriteRenderRequest, SkeletalSpriteRenderRequest::MAX_SPRITE_REQUESTS> SkeletalSpriteRenderRequest::requests;
std::mutex SkeletalSpriteRenderRequest::requestsMutex;

SkeletalSpriteRenderer::SkeletalSpriteRenderer()
{
	instance = this;

	CreateBuffers();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorPool();
	CreateDescriptorSets();
}

SkeletalSpriteRenderer::~SkeletalSpriteRenderer()
{
}

SkeletalSpriteRenderer* SkeletalSpriteRenderer::Get()
{
	return instance;
}

void SkeletalSpriteRenderer::AddSkeletalSprite(SkeletalSprite& sprite, ScreenCoordinate centerPosition, float scale)
{
	SkeletalSpriteRenderRequest* request = SkeletalSpriteRenderRequest::CreateRequest();
	request->vertices = ComputeVertices(sprite, centerPosition, scale);
	request->texture = sprite.texture;
	request->indices = ComputeIndices(sprite);
}

void SkeletalSpriteRenderer::RenderSkeletalSpriteRequest(SkeletalSpriteRenderRequest* request)
{
	currentTexture = request->texture;
	indicesCount = request->indices.size();
	verticesCount = request->vertices.size();

	UpdateDescriptorSets();
	DispatchCommands(request->vertices, request->indices);

	currentIndex = (currentIndex + 1) % SkeletalSpriteRenderer::MAX_REQUESTS_IN_FLIGHT;

	currentTexture = nullptr;
	indicesCount = 0;
	verticesCount = 0;
}

void SkeletalSpriteRenderer::CreateBuffers()
{
	ZoneScoped;
	for (int i = 0; i < MAX_REQUESTS_IN_FLIGHT; i++)
	{
		VkDeviceSize vertexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(SkeletalSpriteVertex);
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

void SkeletalSpriteRenderer::CreatePipeline()
{
	ZoneScoped;
	Shader vertexShader = Shader("Data/Shaders/skeletal_sprite_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragmentShader = Shader("Data/Shaders/skeletal_sprite_frag.spv", Device::Get()->GetVulkanDevice());

	pipeline.SetShader(vertexShader, ShaderType::Vertex);
	pipeline.SetShader(fragmentShader, ShaderType::Fragment);

	pipeline.SetDescriptorSet(descriptorSetLayout);

	pipeline.SetAttributes(SkeletalSpriteVertex::GetAttributeDescriptions());
	pipeline.SetBinding(SkeletalSpriteVertex::GetBindingDescription());
	pipeline.SetColorBlendingEnabled(true);
	pipeline.SetFrontFace(false);

	pipeline.Create();
}

void SkeletalSpriteRenderer::CreateDescriptorSetLayout()
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

void SkeletalSpriteRenderer::CreateDescriptorPool()
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

void SkeletalSpriteRenderer::CreateDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VULKAN_CALL(vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, &descriptorSet));
}

void SkeletalSpriteRenderer::UpdateDescriptorSets()
{
	ZoneScoped;
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

void SkeletalSpriteRenderer::DispatchCommands(std::vector<SkeletalSpriteVertex>& vertices, std::vector<unsigned int>& indices)
{
	ZoneScoped;
	VkCommandBuffer commandBuffer = commandBuffers[currentIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VULKAN_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = Device::Get()->GetRenderPass();
	renderPassInfo.framebuffer = Device::Get()->GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = Device::Get()->GetCurrentExtent();

	VkClearValue clearColor = { {{1.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	Device::Get()->PopulateBufferFromVector(vertices, vertexBuffers[currentIndex], vertexBufferMemories[currentIndex], commandBuffer);
	Device::Get()->PopulateBufferFromVector(indices, indexBuffers[currentIndex], indexBufferMemories[currentIndex], commandBuffer);

	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = vertexBuffers[currentIndex];
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,         // srcStageMask
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,     // dstStageMask
		0,
		0, nullptr,                             // No memory barriers
		1, &barrier,                            // One buffer memory barrier
		0, nullptr                              // No image barriers
	);


	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(Device::Get()->GetCurrentExtent().width);
	viewport.height = static_cast<float>(Device::Get()->GetCurrentExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = Device::Get()->GetCurrentExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer bindableVertexBuffers[] = { vertexBuffers[currentIndex] };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, bindableVertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicesCount), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VULKAN_CALL(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());
}

std::vector<SkeletalSpriteVertex> SkeletalSpriteRenderer::ComputeVertices(SkeletalSprite& sprite, ScreenCoordinate position, float scale)
{
	VkExtent2D screenExtent = Device::Get()->GetCurrentExtent();

	std::vector<SkeletalSpriteVertex> vertices;
	int currentLayer = 0.f;

	for (int i = sprite.layers.size() - 1; i >= 0; i--)
	{
		SkeletalSprite::Layer& layer = sprite.layers[i];
		float layerValue = (float)currentLayer / (float)sprite.layers.size();
		for (SpriteVertex& spriteVertex : layer.vertices)
		{
			glm::vec2 vertexOffset = glm::vec2(spriteVertex.skinnedPosition.x, -spriteVertex.skinnedPosition.y);
			glm::vec2 screenPosition = position.GetScreenPosition() + glm::vec2(vertexOffset.x / screenExtent.width, vertexOffset.y / screenExtent.height) * scale;
			vertices.push_back({ screenPosition, spriteVertex.uv, layerValue });
		}
		
		currentLayer++;
	}

	return vertices;
}

std::vector<unsigned int> SkeletalSpriteRenderer::ComputeIndices(SkeletalSprite& sprite)
{
	std::vector<unsigned int> indices;

	unsigned int previousIndicesCount = 0;

	int currentLayer = 0;

	for (int i = sprite.layers.size() - 1; i >= 0; i--)
	{
		SkeletalSprite::Layer& layer = sprite.layers[i];
		for (int index : layer.indices)
		{
			indices.push_back(index + previousIndicesCount);
		}

		previousIndicesCount += layer.vertices.size();
		currentLayer++;
	}

	return indices;
}

SkeletalSpriteVertex::SkeletalSpriteVertex(glm::vec2 _position, glm::vec2 _uvCoordinate, float _layer)
	: position(_position) , uvCoordinate(_uvCoordinate), layer(_layer)
{
}

VkVertexInputBindingDescription SkeletalSpriteVertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(SkeletalSpriteVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> SkeletalSpriteVertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkVertexInputAttributeDescription positionDescription{};
	positionDescription.binding = 0;
	positionDescription.location = 0;
	positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
	positionDescription.offset = offsetof(SkeletalSpriteVertex, position);

	VkVertexInputAttributeDescription uvCoordinateDescription{};
	uvCoordinateDescription.binding = 0;
	uvCoordinateDescription.location = 1;
	uvCoordinateDescription.format = VK_FORMAT_R32G32_SFLOAT;
	uvCoordinateDescription.offset = offsetof(SkeletalSpriteVertex, uvCoordinate);

	VkVertexInputAttributeDescription layerDescription{};
	layerDescription.binding = 0;
	layerDescription.location = 2;
	layerDescription.format = VK_FORMAT_R32_SFLOAT;
	layerDescription.offset = offsetof(SkeletalSpriteVertex, layer);

	attributeDescriptions.push_back(positionDescription);
	attributeDescriptions.push_back(uvCoordinateDescription);
	attributeDescriptions.push_back(layerDescription);

	return attributeDescriptions;
}

bool SkeletalSpriteRenderRequest::CanBeCombined(const RenderRequest* other) const
{
	return false;
}

void SkeletalSpriteRenderRequest::CombineWith(RenderRequest* other)
{
}

void SkeletalSpriteRenderRequest::Render()
{
	ZoneScopedN("ImageRenderRequest::Render");
	SkeletalSpriteRenderer::Get()->RenderSkeletalSpriteRequest(this);
}

void SkeletalSpriteRenderRequest::Clean()
{
	vertices.clear();
}

SkeletalSpriteRenderRequest* SkeletalSpriteRenderRequest::CreateRequest()
{
	requestsMutex.lock();

	if (!requestsArrayInitialized)
	{
		for (int i = 0; i < MAX_SPRITE_REQUESTS; i++)
		{
			requests[i].isActive = false;
		}
		requestsArrayInitialized = true;
	}

	lastIndex = (lastIndex + 1) % MAX_SPRITE_REQUESTS;
	requests[lastIndex].isActive = true;
	requestsMutex.unlock();

	return &requests[lastIndex];
}

std::vector<RenderRequest*> SkeletalSpriteRenderRequest::GetRequestsThisFrame()
{
	std::vector<RenderRequest*> requestsThisFrame;

	for (SkeletalSpriteRenderRequest& request : requests)
	{
		if (request.isActive && !request.isProcessing)
		{
			requestsThisFrame.push_back(&request);
		}
	}

	return requestsThisFrame;
}
