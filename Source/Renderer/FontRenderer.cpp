#include "FontRenderer.h"

#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"

#include "AssertUtils.h"

#include "tracy/Tracy.hpp"

const std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

std::array<FontRenderRequest, FontRenderRequest::MAX_FONT_REQUESTS> FontRenderRequest::requests;
std::mutex FontRenderRequest::requestsMutex;

const float letterSpacingFactor = 0.2f;
const float spaceSpacingFactor = 12.0f;

bool FontRenderRequest::CanBeCombined(const RenderRequest* other) const
{
	// Right now, we only merge requests with the same font
	if (const FontRenderRequest* otherFont = dynamic_cast<const FontRenderRequest*>(other))
	{
		if (otherFont->font != font)
		{
			return false;
		}

		return true;
	}

	return false;
}

void FontRenderRequest::CombineWith(RenderRequest* other)
{
	if (const FontRenderRequest* otherFontRequest = dynamic_cast<const FontRenderRequest*>(other))
	{
		for (int i = 0; i < otherFontRequest->texts.size(); i++)
		{
			texts.push_back(otherFontRequest->texts[i]);
		}
	}
}

void FontRenderRequest::Render()
{
	ZoneScopedN("FontRenderRequest::Render");
	FontRenderer::Get()->RenderFontRequest(this);
}

void FontRenderRequest::Clean()
{
	texts.clear();
}

FontRenderRequest* FontRenderRequest::CreateRequest()
{
	static const int DEFAULT_REQUESTS_RESERVE_SIZE = 8;

	requestsMutex.lock();

	if (!requestsArrayInitialized)
	{
		for (int i = 0; i < MAX_FONT_REQUESTS; i++)
		{
			requests[i].isActive = false;
			requests[i].texts.reserve(DEFAULT_REQUESTS_RESERVE_SIZE);
		}
		requestsArrayInitialized = true;
	}

	lastIndex = (lastIndex + 1) % MAX_FONT_REQUESTS;
	requests[lastIndex].isActive = true;
	requestsMutex.unlock();

	return &requests[lastIndex];
}

std::vector<RenderRequest*> FontRenderRequest::GetRequestsThisFrame()
{
	std::vector<RenderRequest*> requestsThisFrame;

	for (FontRenderRequest& request : requests)
	{
		if (request.isActive && !request.isProcessing)
		{
			requestsThisFrame.push_back(&request);
		}
	}

	return requestsThisFrame;
}

FontRenderer::FontRenderer()
{
	instance = this;

	defaultFont = new Font("Data/Fonts/Roboto-Medium.ttf", {60});

	Device::Get()->TransitionImageLayout(
		defaultFont->texture->GetImage(),
		VK_FORMAT_R8G8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	for (int i = 0; i < MAX_REQUESTS_IN_FLIGHT; i++)
	{
		VkDeviceSize vertexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(FontVertex);
		Device::Get()->CreateBuffer(vertexBufferSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			fontVertexBuffers[i], 
			fontVertexBufferMemories[i]);

		VkDeviceSize indexBufferSize = MAX_VERTICES_IN_REQUEST * sizeof(unsigned int);
		Device::Get()->CreateBuffer(indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			fontIndexBuffers[i],
			fontIndexBufferMemories[i]);
	}

	CreatePipeline();
	CreateCommandBuffers();
}

FontRenderer::~FontRenderer()
{
	delete defaultFont;

	vkFreeCommandBuffers(Device::Get()->GetVulkanDevice(), Device::Get()->GetCommandPool(), commandBuffers.size(), commandBuffers.data());
}

FontRenderer* FontRenderer::Get()
{
	return instance;
}

void FontRenderer::AddText(std::string text, glm::vec2 position, float renderingOrder, int fontSize)
{
	ASSERT(defaultFont != nullptr);

	if (text.empty())
		return;

	FontRenderRequest* request = FontRenderRequest::CreateRequest();
	request->texts.push_back({ text, position, fontSize });
	request->renderingOrder = renderingOrder;

	// TODO: Allow suggestions to override default font
	request->font = defaultFont;
}

void FontRenderer::RenderFontRequest(FontRenderRequest* request)
{
	for (FontRenderRequest::TextRequest& textRequest : request->texts)
	{
		PopulateBufferWithTextRequest(textRequest);
	}

	UpdateDescriptorSets();
	DispatchCommands();

	indices.clear();
	vertices.clear();

	currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
}

void FontRenderer::PopulateBufferWithTextRequest(FontRenderRequest::TextRequest& request)
{
	ZoneScoped;
	int width, height;
	Window::GetWindowSize(&width, &height);
	float normalizedFontSize = (static_cast<float>(request.fontSize) * 6.333f) / width;

	glm::vec2 currentCursorLocation = request.position;
	Font* font = defaultFont;
	float fontScale = font->GetPixelScale(static_cast<float>(request.fontSize));

	for (int i = 0; i < request.text.size(); i++)
	{
		char c = request.text[i];

		if (c == 32)
		{
			currentCursorLocation += glm::vec2(spaceSpacingFactor * (fontScale / width), 0);
			continue;
		}

		int charIndex = font->GetCharacterIndex(c);
		if (charIndex == -1)
		{
			// TODO: Font Renderer Error Message
			continue;
		}

		Font::Character& character = font->characters[charIndex];
		float uvWidth = character.uvCoordinates.right - character.uvCoordinates.left;
		float normalizedCharWidth = uvWidth * font->GetBitmapWidth() * fontScale / width;

		float uvHeight = character.uvCoordinates.top - character.uvCoordinates.bottom;
		float normalizedCharHeight = uvHeight * font->GetBitmapHeight() * fontScale / height;
		glm::vec2 normalizedOffset = glm::vec2(
			character.offset.x * (fontScale / width),
			character.offset.y * (fontScale / height));

		float yOffset = normalizedOffset.y;

		float bottom = currentCursorLocation.y - yOffset;
		float left = currentCursorLocation.x + normalizedOffset.x;
		float top = currentCursorLocation.y + normalizedCharHeight - yOffset;
		float right = left + normalizedCharWidth;

		float uvBottom = character.uvCoordinates.bottom;
		float uvLeft = character.uvCoordinates.left;
		float uvTop = character.uvCoordinates.top;
		float uvRight = character.uvCoordinates.right;

		vertices.push_back(FontVertex(glm::vec2(left, bottom), glm::vec2(uvLeft, uvBottom)));
		vertices.push_back(FontVertex(glm::vec2(right, top), glm::vec2(uvRight, uvTop)));
		vertices.push_back(FontVertex(glm::vec2(right, bottom), glm::vec2(uvRight, uvBottom)));
		vertices.push_back(FontVertex(glm::vec2(left, top), glm::vec2(uvLeft, uvTop)));

		unsigned int vertexIndex = indices.size() / 6;

		indices.push_back(vertexIndex * 4 + 1);
		indices.push_back(vertexIndex * 4 + 2);
		indices.push_back(vertexIndex * 4 + 3);
		indices.push_back(vertexIndex * 4 + 2);
		indices.push_back(vertexIndex * 4 + 0);
		indices.push_back(vertexIndex * 4 + 3);

		currentCursorLocation.x += normalizedCharWidth + character.xAdvance * letterSpacingFactor * (fontScale / width);
	}
}

void FontRenderer::CreatePipeline()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	VULKAN_CALL(vkCreateDescriptorSetLayout(Device::Get()->GetVulkanDevice(), &layoutInfo, nullptr, &descriptorSetLayout));

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VULKAN_CALL(vkCreateDescriptorPool(Device::Get()->GetVulkanDevice(), &poolInfo, nullptr, &descriptorPool));

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	VULKAN_CALL(vkAllocateDescriptorSets(Device::Get()->GetVulkanDevice(), &allocInfo, descriptorSets.data()));

	Shader vertexShader = Shader("C:\\Users\\debgh\\source\\repos\\Project Multiverse\\Data\\Shaders\\font_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragmentShader = Shader("C:\\Users\\debgh\\source\\repos\\Project Multiverse\\Data\\Shaders\\font_frag.spv", Device::Get()->GetVulkanDevice());

	pipeline.SetShader(vertexShader, ShaderType::Vertex);
	pipeline.SetShader(fragmentShader, ShaderType::Fragment);

	pipeline.SetDescriptorSet(descriptorSetLayout);

	pipeline.SetAttributes(FontVertex::GetAttributeDescriptions());
	pipeline.SetBinding(FontVertex::GetBindingDescription());
	pipeline.SetColorBlendingEnabled(true);

	pipeline.Create();
}

void FontRenderer::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = Device::Get()->GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	VULKAN_CALL(vkAllocateCommandBuffers(Device::Get()->GetVulkanDevice(), &allocInfo, commandBuffers.data()));
}

void FontRenderer::UpdateDescriptorSets()
{
	ZoneScoped;
	std::array<VkWriteDescriptorSet, 1> setWrites;

	VkDescriptorImageInfo samplerInfo = {};
	samplerInfo.sampler = defaultFont->texture->GetSampler();
	samplerInfo.imageView = defaultFont->texture->GetImageView();
	samplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	setWrites[0] = {};
	setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[0].dstBinding = 0;
	setWrites[0].dstArrayElement = 0;
	setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setWrites[0].descriptorCount = 1;
	setWrites[0].dstSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
	setWrites[0].pImageInfo = &samplerInfo;

	vkUpdateDescriptorSets(Device::Get()->GetVulkanDevice(), 1, &setWrites[0], 0, nullptr);
}

void FontRenderer::DispatchCommands()
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

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	Device::Get()->PopulateBufferFromVector(vertices, fontVertexBuffers[currentIndex], fontIndexBufferMemories[currentIndex], commandBuffer);
	Device::Get()->PopulateBufferFromVector(indices, fontIndexBuffers[currentIndex], fontIndexBufferMemories[currentIndex], commandBuffer);

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

	VkBuffer vertexBuffers[] = { fontVertexBuffers[currentIndex]};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, fontIndexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT32);

	VkDescriptorSet& descriptorSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
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

FontVertex::FontVertex(glm::vec2 _position, glm::vec2 _uvCoordinate)
{
	position = _position;
	uvCoordinate = _uvCoordinate;
}

VkVertexInputBindingDescription FontVertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(FontVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> FontVertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkVertexInputAttributeDescription positionDescription{};
	positionDescription.binding = 0;
	positionDescription.location = 0;
	positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
	positionDescription.offset = offsetof(FontVertex, position);

	VkVertexInputAttributeDescription uvCoordinateDescription{};
	uvCoordinateDescription.binding = 0;
	uvCoordinateDescription.location = 1;
	uvCoordinateDescription.format = VK_FORMAT_R32G32_SFLOAT;
	uvCoordinateDescription.offset = offsetof(FontVertex, uvCoordinate);

	attributeDescriptions.push_back(positionDescription);
	attributeDescriptions.push_back(uvCoordinateDescription);

	return attributeDescriptions;
}
