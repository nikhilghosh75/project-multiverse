#include "FontRenderer.h"

#include "Device.h"
#include "VulkanUtils.h"
#include "Window.h"

const std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

std::array<FontRenderRequest, FontRenderRequest::MAX_FONT_REQUESTS> FontRenderRequest::requests;

const float letterSpacingFactor = 0.2f;
const float spaceSpacingFactor = 12.0f;

bool FontRenderRequest::CanBeCombined(const RenderRequest* other) const
{
	if (other->type != RenderRequestType::Font)
	{
		return false;
	}

	// TODO: Add font merging
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
	if (const FontRenderRequest* otherFont = dynamic_cast<const FontRenderRequest*>(other))
	{

	}
}

void FontRenderRequest::Render()
{
}

FontRenderRequest* FontRenderRequest::CreateRequest()
{
	return nullptr;
}

FontRenderer::FontRenderer()
{
	instance = this;

	defaultFont = new Font("Data/Fonts/Roboto-Medium.ttf");

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

	vkDestroyPipeline(Device::Get()->GetVulkanDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Device::Get()->GetVulkanDevice(), pipelineLayout, nullptr);

	vkFreeCommandBuffers(Device::Get()->GetVulkanDevice(), Device::Get()->GetCommandPool(), commandBuffers.size(), commandBuffers.data());
}

FontRenderer* FontRenderer::Get()
{
	return instance;
}

void FontRenderer::AddText(std::string text, glm::vec2 position, int fontSize)
{
	if (text.empty())
		return;

	int width, height;
	Window::GetWindowSize(&width, &height);
	float normalizedFontSize = (static_cast<float>(fontSize) * 6.333f) / width;

	glm::vec2 currentCursorLocation = position;
	Font* font = defaultFont;
	float fontScale = font->GetPixelScale(static_cast<float>(fontSize));

	for (int i = 0; i < text.size(); i++)
	{
		char c = text[i];

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
		// glm::vec2 normalizedOffset = glm::vec2(0, 0);

		float yOffset = normalizedOffset.y;
		if (c == 'p' || c == 'g' || c == 'q' || c == 'j')
		{
			// yOffset = -0.021f * fontScale;
		}

		float bottom = currentCursorLocation.y - normalizedCharHeight - yOffset;
		float left = currentCursorLocation.x + normalizedOffset.x;
		float top = currentCursorLocation.y - yOffset;
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

	Render();
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

	Shader vertexShader = Shader::ReadShader("C:\\Users\\debgh\\source\\repos\\Project Multiverse\\Data\\Shaders\\font_vert.spv", Device::Get()->GetVulkanDevice());
	Shader fragShader = Shader::ReadShader("C:\\Users\\debgh\\source\\repos\\Project Multiverse\\Data\\Shaders\\font_frag.spv", Device::Get()->GetVulkanDevice());

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VULKAN_CALL(vkCreatePipelineLayout(Device::Get()->GetVulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout))

	VkDevice vulkanDevice = Device::Get()->GetVulkanDevice();
	VkExtent2D swapChainExtent = Device::Get()->GetSwapChainExtent();

	// Setups the vertex shader as a code-based shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader.GetModule();
	vertShaderStageInfo.pName = "main";

	// Setups the fragment shader as a code-based shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShader.GetModule();
	fragShaderStageInfo.pName = "main";

	// Setup the shader stages
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkVertexInputBindingDescription bindingDescription = FontVertex::GetBindingDescription();
	std::vector<VkVertexInputAttributeDescription> attributeDescription = FontVertex::GetAttributeDescriptions();

	// Describes the structure of the vertex data
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	// Describes what types of primatives (point, line, etc.)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Describes the region of the framebuffer that will be used
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Describes the region that will be cut off
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	// ?
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Setup the rasterizer (draws the pixels on screen)
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Setup multisampling (for antialiasing)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Determines which parts of the pipeline can be changed without recreating the pipeline
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Setup color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = Device::Get()->GetRenderPass();
	pipelineInfo.subpass = 0;

	VULKAN_CALL_MSG(vkCreateGraphicsPipelines(vulkanDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline), "Failed to create graphics pipeline");
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

void FontRenderer::PopulateBuffers()
{
	Device::Get()->PopulateBufferFromVector(vertices, fontVertexBuffers[currentIndex], fontIndexBufferMemories[currentIndex]);
	Device::Get()->PopulateBufferFromVector(indices, fontIndexBuffers[currentIndex], fontIndexBufferMemories[currentIndex]);
}

void FontRenderer::DispatchCommands()
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

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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

	VkBuffer vertexBuffers[] = { fontVertexBuffers[currentIndex]};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, fontIndexBuffers[currentIndex], 0, VK_INDEX_TYPE_UINT32);

	VkDescriptorSet& descriptorSet = descriptorSets[Device::GetFrameNumber() % MAX_FRAMES_IN_FLIGHT];
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

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

void FontRenderer::Render()
{
	if (Device::Get()->shouldRenderFrame)
	{
		PopulateBuffers();
		UpdateDescriptorSets();
		DispatchCommands();

		indices.clear();
		vertices.clear();

		currentIndex = (currentIndex + 1) % MAX_REQUESTS_IN_FLIGHT;
	}
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
