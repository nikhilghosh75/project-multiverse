#include "RenderPipeline.h"
#include "Device.h"
#include "VulkanUtils.h"

const std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

RenderPipeline::RenderPipeline()
{
    bindingDescription = VkVertexInputBindingDescription();
}

RenderPipeline::RenderPipeline(VkVertexInputBindingDescription _binding, std::vector<VkVertexInputAttributeDescription> _attributes)
{
    bindingDescription = _binding;
    attributeDescription = _attributes;
}

RenderPipeline::~RenderPipeline()
{
    vkDestroyPipelineLayout(Device::Get()->GetVulkanDevice(), pipelineLayout, nullptr);
}

void RenderPipeline::SetShader(Shader& shader, ShaderType type)
{
    shaders.emplace_back(shader.GetModule(), type);
}

void RenderPipeline::SetDescriptorSet(VkDescriptorSetLayout layout)
{
    descriptorLayout = layout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorLayout;

    VULKAN_CALL(vkCreatePipelineLayout(Device::Get()->GetVulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));
}

void RenderPipeline::SetBinding(VkVertexInputBindingDescription _binding)
{
    bindingDescription = _binding;
}

void RenderPipeline::SetAttributes(std::vector<VkVertexInputAttributeDescription> _attributes)
{
    attributeDescription = _attributes;
}

void RenderPipeline::SetColorBlendingEnabled(bool enabled)
{
    colorBlendingEnabled = enabled;
}

void RenderPipeline::SetTopology(VkPrimitiveTopology topology)
{
    primitiveTopology = topology;
}

void RenderPipeline::SetPolygonMode(VkPolygonMode mode)
{
    polygonMode = mode;
}

void RenderPipeline::Create()
{
    VkDevice vulkanDevice = Device::Get()->GetVulkanDevice();
    VkExtent2D swapChainExtent = Device::Get()->GetCurrentExtent();

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaders(shaders.size());

    for (int i = 0; i < shaders.size(); i++)
    {
        pipelineShaders[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaders[i].pName = "main";
        pipelineShaders[i].stage = GetShaderStage(shaders[i].type);
        pipelineShaders[i].module = shaders[i].shader;
    }

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
    inputAssembly.topology = primitiveTopology;
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
    rasterizer.polygonMode = polygonMode;
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

    if (colorBlendingEnabled)
    {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    else
    {
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = pipelineShaders.size();
    pipelineInfo.pStages = pipelineShaders.data();
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

    VULKAN_CALL_MSG(vkCreateGraphicsPipelines(vulkanDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline), "Vulkan, failed to create graphics pipelines");
}

VkPipeline RenderPipeline::GetPipeline() const
{
    return graphicsPipeline;
}

VkPipelineLayout RenderPipeline::GetPipelineLayout() const
{
    return pipelineLayout;
}

VkShaderStageFlagBits RenderPipeline::GetShaderStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
    }

    return (VkShaderStageFlagBits)0x00;
}

RenderPipeline::ShaderData::ShaderData(VkShaderModule _shader, ShaderType _type)
    : shader(_shader), type(_type)
{
}
