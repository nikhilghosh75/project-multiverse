#include "DebugRenderer.h"
#include "Device.h"

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

	Render();
}

void DebugRenderer::CreatePipeline()
{
	Shader vertexShader = Shader::ReadShader("Data/Shaders/debug_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragmentShader = Shader::ReadShader("Data/Shaders/debug_frag.spv", Device::Get()->GetVulkanDevice());

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

	if (vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		exit(0);
	}
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

	if (vkCreateDescriptorPool(Device::Get()->GetVulkanDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		exit(0);
	}
}

void DebugRenderer::CreateDescriptorSets()
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

void DebugRenderer::UpdateDescriptorSets()
{

}

void DebugRenderer::PopulateBuffers()
{
	Device::Get()->PopulateBufferFromVector(vertices, vertexBuffers[currentIndex], vertexBufferMemories[currentIndex]);
	Device::Get()->PopulateBufferFromVector(indices, indexBuffers[currentIndex], indexBufferMemories[currentIndex]);
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

	VkBuffer bindableVertexBuffers[] = { vertexBuffers[currentIndex] };
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
	indices.clear();

	currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void DebugRenderer::Render()
{
	UpdateDescriptorSets();
	PopulateBuffers();
	DispatchCommands();
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