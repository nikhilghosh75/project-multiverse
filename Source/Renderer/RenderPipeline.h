#pragma once
#include <vector>
#include "vulkan/vulkan.h"
#include "Shader.h"

class RenderPipeline
{
public:
	RenderPipeline();
	RenderPipeline(VkVertexInputBindingDescription _binding, std::vector<VkVertexInputAttributeDescription> _attributes);

	void SetShader(Shader& shader, ShaderType type);
	void SetDescriptorSet(VkDescriptorSetLayout layout);
	void SetBinding(VkVertexInputBindingDescription _binding);
	void SetAttributes(std::vector<VkVertexInputAttributeDescription> _attributes);

	void Create();

	VkPipeline GetPipeline() const;
	VkPipelineLayout GetPipelineLayout() const;

private:
	VkShaderStageFlagBits GetShaderStage(ShaderType type);

	class ShaderData
	{
	public:
		VkShaderModule shader;
		ShaderType type;

		ShaderData(VkShaderModule _shader, ShaderType _type);
	};

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescription;
	VkDescriptorSetLayout descriptorLayout;

	std::vector<ShaderData> shaders;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	bool hasDescriptorLayout = false;
};