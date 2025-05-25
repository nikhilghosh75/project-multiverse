#include "DebugRenderer.h"

#include "Device.h"
#include "VulkanUtils.h"

#include "tracy/Tracy.hpp"

std::array<DebugRenderRequest, DebugRenderRequest::MAX_DEBUG_REQUESTS> DebugRenderRequest::requests;
std::mutex DebugRenderRequest::requestsMutex;

bool DebugRenderRequest::CanBeCombined(const RenderRequest* other) const
{
	if (const DebugRenderRequest* otherDebugRequest = dynamic_cast<const DebugRenderRequest*>(other))
	{
		return true;
	}

	return false;
}

void DebugRenderRequest::CombineWith(RenderRequest* other)
{
	if (const DebugRenderRequest* otherDebugRequest = dynamic_cast<const DebugRenderRequest*>(other))
	{
		for (int i = 0; i < otherDebugRequest->rects.size(); i++)
		{
			rects.push_back(otherDebugRequest->rects[i]);
		}
	}
}

void DebugRenderRequest::Render()
{
	ZoneScopedN("DebugRenderRequest::Render");
	DebugRenderer::Get()->RenderDebugRequest(this);
}

void DebugRenderRequest::Clean()
{
	rects.clear();
}

DebugRenderRequest* DebugRenderRequest::CreateRequest()
{
	static const int DEFAULT_RECTS_RESERVE_SIZE = 8;

	requestsMutex.lock();

	if (!requestsArrayInitialized)
	{
		for (int i = 0; i < MAX_DEBUG_REQUESTS; i++)
		{
			requests[i].isActive = false;
			requests[i].rects.reserve(DEFAULT_RECTS_RESERVE_SIZE);
		}
		requestsArrayInitialized = true;
	}

	lastIndex = (lastIndex + 1) % MAX_DEBUG_REQUESTS;
	requests[lastIndex].isActive = true;
	requestsMutex.unlock();

	return &requests[lastIndex];
}

std::vector<RenderRequest*> DebugRenderRequest::GetRequestsThisFrame()
{
	std::vector<RenderRequest*> requestsThisFrame;

	for (DebugRenderRequest& request : requests)
	{
		if (request.isActive && !request.isProcessing)
		{
			requestsThisFrame.push_back(&request);
		}
	}

	return requestsThisFrame;
}

DebugRenderer::DebugRenderer()
{
	instance = this;

	for (int i = 0; i < MAX_REQUESTS_IN_FLIGHT; i++)
	{
		VkDeviceSize vertexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(DebugVertex);
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

	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorPool();
	CreateDescriptorSets();
}

DebugRenderer::~DebugRenderer()
{
}

DebugRenderer* DebugRenderer::Get()
{
	return instance;
}

void DebugRenderer::AddBox(Rect rect)
{
	DebugRenderRequest* request = DebugRenderRequest::CreateRequest();
	request->rects.push_back(rect);
}

void DebugRenderer::RenderDebugRequest(DebugRenderRequest* request)
{
	for (int i = 0; i < request->rects.size(); i++)
	{
		PopulateWithBox(request->rects[i]);
	}

	UpdateDescriptorSets();
	DispatchCommands();

	vertices.clear();
	indices.clear();

	currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void DebugRenderer::PopulateWithBox(Rect rect)
{
	glm::vec2 bottomLeft = glm::vec2(rect.left, rect.bottom);
	glm::vec2 topLeft = glm::vec2(rect.left, rect.top);
	glm::vec2 topRight = glm::vec2(rect.right, rect.top);
	glm::vec2 bottomRight = glm::vec2(rect.right, rect.bottom);

	DebugVertex bottomLeftVertex(bottomLeft);
	DebugVertex topLeftVertex(topLeft);
	DebugVertex topRightVertex(topRight);
	DebugVertex bottomRightVertex(bottomRight);

	vertices.push_back(bottomLeftVertex);
	vertices.push_back(topLeftVertex);
	vertices.push_back(topRightVertex);
	vertices.push_back(bottomRightVertex);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
}

void DebugRenderer::CreatePipeline()
{
	Shader vertexShader = Shader("Data/Shaders/debug_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragmentShader = Shader("Data/Shaders/debug_frag.spv", Device::Get()->GetVulkanDevice());

	pipeline.SetShader(vertexShader, ShaderType::Vertex);
	pipeline.SetShader(fragmentShader, ShaderType::Fragment);

	pipeline.SetDescriptorSet(descriptorSetLayout);

	pipeline.SetAttributes(DebugVertex::GetAttributeDescriptions());
	pipeline.SetBinding(DebugVertex::GetBindingDescription());

	pipeline.SetColorBlendingEnabled(true);
	pipeline.SetPolygonMode(VK_POLYGON_MODE_LINE);
	pipeline.SetTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	pipeline.Create();
}

void DebugRenderer::CreateDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 0> bindings = { };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VULKAN_CALL(vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout));
}

void DebugRenderer::CreateDescriptorPool()
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

void DebugRenderer::CreateDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VULKAN_CALL(vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, &descriptorSet));
}

void DebugRenderer::UpdateDescriptorSets()
{

}

void DebugRenderer::DispatchCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = Device::Get()->GetCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, &commandBuffer);

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

	vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT16);

	// VkDescriptorSet& descriptorSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VULKAN_CALL_MSG(vkEndCommandBuffer(commandBuffer), "Failed to end command buffer");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(Device::Get()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(Device::Get()->GetGraphicsQueue());
}

DebugRenderer::DebugVertex::DebugVertex(glm::vec2 position)
	: position(position)
{
}

VkVertexInputBindingDescription DebugRenderer::DebugVertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(DebugVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> DebugRenderer::DebugVertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkVertexInputAttributeDescription positionDescription{};
	positionDescription.binding = 0;
	positionDescription.location = 0;
	positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
	positionDescription.offset = offsetof(DebugVertex, position);

	attributeDescriptions.push_back(positionDescription);

	return attributeDescriptions;
}

