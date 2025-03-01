#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <string>

enum class ShaderType
{
	None,
	Vertex,
	Fragment
};

class Shader
{
public:
	Shader(const std::string& fileName, VkDevice device);

	~Shader();

	VkShaderModule GetModule() const { return module; };

private:
	uint32_t size;
	char* data;
	VkShaderModule module;
};

