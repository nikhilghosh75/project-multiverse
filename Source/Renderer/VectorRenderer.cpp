#include "VectorRenderer.h"

#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"

#include "tracy/Tracy.hpp"

std::array<SimpleVectorRenderRequest, SimpleVectorRenderRequest::MAX_VECTOR_REQUESTS> SimpleVectorRenderRequest::requests;

VectorPainter::VectorPainter()
	: state(State::WaitingToBegin), screenSpace(ScreenSpace::Screen)
{

}

VectorPainter::VectorPainter(ScreenSpace space)
	: state(State::WaitingToBegin), screenSpace(space)
{
}

void VectorPainter::SetFillColor(Color color)
{
	currentColor = color;
}

void VectorPainter::BeginPath()
{
	if (state != State::WaitingToBegin)
	{
		ClosePath();
	}

	paths.push_back(Path());
	paths[paths.size() - 1].color = currentColor;

	state = State::WaitingForStartingPoint;
}

void VectorPainter::ClosePath()
{
	state = State::WaitingToBegin;
}

void VectorPainter::PointTo(glm::vec2 point)
{
	if (state == State::WaitingForStartingPoint)
	{
		paths[paths.size() - 1].points.push_back(point);
		state == State::Painting;
	}
	else
	{
		LineTo(point);
	}
}

void VectorPainter::LineTo(glm::vec2 point)
{
	paths[paths.size() - 1].points.push_back(point);
}

void VectorPainter::DrawRegularPolygon(glm::vec2 center, int sides, float radius, float angle /* in radians*/)
{
	paths.push_back(Path());
	paths[paths.size() - 1].color = currentColor;

	float deltaAngle = 3.141592f * 2.f / sides;

	int screenWidth, screenHeight;
	Window::GetWindowSize(&screenWidth, &screenHeight);

	float radiusX = radius * (float)screenHeight / (float)screenWidth;
	float radiusY = radius;

	glm::vec2 startPoint = glm::vec2(
		center.x + radiusX * std::cos(-angle),
		center.y + radiusY * std::sin(-angle)
	);
	paths[paths.size() - 1].points.push_back(startPoint);

	for (int i = 1; i < sides; i++)
	{
		glm::vec2 point = glm::vec2(
			center.x + radiusX * std::cos(deltaAngle * i - angle),
			center.y + radiusY * std::sin(deltaAngle * i - angle)
		);
		paths[paths.size() - 1].points.push_back(point);
	}

	state = State::WaitingToBegin;
}

bool SimpleVectorRenderRequest::CanBeCombined(const RenderRequest* other) const
{
	if (const SimpleVectorRenderRequest* vectorRequest = dynamic_cast<const SimpleVectorRenderRequest*>(other))
	{
		// TODO: Allow vector requests of two different spaces to be combined
		return vectorRequest->space == space;
	}
	return false;
}

void SimpleVectorRenderRequest::CombineWith(RenderRequest* other)
{
	if (SimpleVectorRenderRequest* otherRequest = dynamic_cast<SimpleVectorRenderRequest*>(other))
	{
		for (VectorPainter::Path& path : otherRequest->paths)
		{
			paths.push_back(path);
		}
	}
}

void SimpleVectorRenderRequest::Render(VkCommandBuffer commandBuffer)
{
	ZoneScopedN("VectorRenderRequest::Render");
	VectorRenderer::Get()->RenderSimpleVectorRequest(this, commandBuffer);
}

void SimpleVectorRenderRequest::Clean()
{
	paths.clear();
}

SimpleVectorRenderRequest* SimpleVectorRenderRequest::CreateRequest()
{
	if (!requestsArrayInitialized)
	{
		for (int i = 0; i < MAX_VECTOR_REQUESTS; i++)
		{
			requests[i].isActive = false;
		}
		requestsArrayInitialized = true;
	}

	lastIndex = (lastIndex + 1) % MAX_VECTOR_REQUESTS;
	requests[lastIndex].isActive = true;
	return &requests[lastIndex];
}

std::vector<RenderRequest*> SimpleVectorRenderRequest::GetRequestsThisFrame()
{
	std::vector<RenderRequest*> requestsThisFrame;

	for (SimpleVectorRenderRequest& request : requests)
	{
		if (request.isActive && !request.isProcessing)
		{
			requestsThisFrame.push_back(&request);
		}
	}

	return requestsThisFrame;
}

VectorRenderer::VectorRenderer()
{
	instance = this;

	CreateBuffers();
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateDescriptorPool();
	CreateDescriptorSets();
}

VectorRenderer::~VectorRenderer()
{
	vkFreeCommandBuffers(Device::Get()->GetVulkanDevice(), Device::Get()->GetCommandPool(), commandBuffers.size(), commandBuffers.data());
}

VectorRenderer* VectorRenderer::Get()
{
	return instance;
}

void VectorRenderer::SubmitPainter(const VectorPainter& painter)
{
	SimpleVectorRenderRequest* request = SimpleVectorRenderRequest::CreateRequest();
	request->space = painter.screenSpace;

	for (int i = 0; i < painter.paths.size(); i++)
	{
		if (IsValidPath(painter.paths[i]))
		{
			request->paths.push_back(painter.paths[i]);
		}
	}
}

void VectorRenderer::RenderSimpleVectorRequest(SimpleVectorRenderRequest* request, VkCommandBuffer commandBuffer)
{
	maxLayers = request->paths.size() + 1;

	for (int i = 0; i < request->paths.size(); i++)
	{
		if (IsValidPath(request->paths[i]))
		{
			SubmitPath(request->paths[i], request->space);
		}
	}

	UpdateDescriptorSets();
	PopulateBuffers();
	DispatchCommands(commandBuffer);

	vertices.clear();
	indices.clear();
	currentLayer = 0;
	maxLayers = 0;
	currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void VectorRenderer::CreateBuffers()
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

void VectorRenderer::CreatePipeline()
{
	Shader vertexShader = Shader("Data/Shaders/vector_simple_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragmentShader = Shader("Data/Shaders/vector_simple_frag.spv", Device::Get()->GetVulkanDevice());

	pipeline.SetShader(vertexShader, ShaderType::Vertex);
	pipeline.SetShader(fragmentShader, ShaderType::Fragment);

	pipeline.SetDescriptorSet(descriptorSetLayout);

	pipeline.SetAttributes(Vertex::GetAttributeDescriptions());
	pipeline.SetBinding(Vertex::GetBindingDescription());

	pipeline.Create();
}

void VectorRenderer::CreateDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 0> bindings = { };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VULKAN_CALL(vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout));
}

void VectorRenderer::CreateDescriptorPool()
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

void VectorRenderer::CreateDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VULKAN_CALL(vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, &descriptorSet));
}

void VectorRenderer::UpdateDescriptorSets()
{
	
}

void VectorRenderer::PopulateBuffers()
{
	Device::Get()->PopulateBufferFromVector(vertices, vertexBuffers[currentIndex], vertexBufferMemories[currentIndex]);
	Device::Get()->PopulateBufferFromVector(indices, indexBuffers[currentIndex], indexBufferMemories[currentIndex]);
}

void VectorRenderer::DispatchCommands(VkCommandBuffer commandBuffer)
{
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

	VkBuffer bindableVertexBuffers[] = { vertexBuffers[currentIndex]};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, bindableVertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT16);

	// VkDescriptorSet& descriptorSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

bool VectorRenderer::IsValidPath(const VectorPainter::Path& path)
{
	return path.points.size() > 0 && path.color.a > 0;
}

void VectorRenderer::SubmitPath(const VectorPainter::Path& path, ScreenSpace space)
{
	uint16_t startIndex = vertices.size();

	// Upload Vertices
	for (int i = 0; i < path.points.size(); i++)
	{
		glm::vec2 point = ScreenCoordinate::ConvertPointBetweenSpace(path.points[i], space, ScreenSpace::Rendering);
		vertices.push_back(Vertex(point, (glm::vec4)path.color, currentLayer / maxLayers));
	}

	// Upload Indices
	for (int i = 1; i < path.points.size() - 1; i++)
	{
		indices.push_back(startIndex);
		indices.push_back(startIndex + i);
		indices.push_back(startIndex + i + 1);
	}

	currentLayer += 1.0f;
}

VectorRenderer::Vertex::Vertex(glm::vec2 _position, glm::vec4 _color, float _layer)
	: position(_position), color(_color), layer(_layer)
{
}

VkVertexInputBindingDescription VectorRenderer::Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VectorRenderer::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkVertexInputAttributeDescription positionDescription;
	positionDescription.binding = 0;
	positionDescription.location = 0;
	positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
	positionDescription.offset = offsetof(Vertex, position);

	VkVertexInputAttributeDescription colorDescription;
	colorDescription.binding = 0;
	colorDescription.location = 1;
	colorDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorDescription.offset = offsetof(Vertex, color);

	VkVertexInputAttributeDescription layerDescription;
	layerDescription.binding = 0;
	layerDescription.location = 2;
	layerDescription.format = VK_FORMAT_R32_SFLOAT;
	layerDescription.offset = offsetof(Vertex, layer);

	attributeDescriptions.push_back(positionDescription);
	attributeDescriptions.push_back(colorDescription);
	attributeDescriptions.push_back(layerDescription);

	return attributeDescriptions;
}
