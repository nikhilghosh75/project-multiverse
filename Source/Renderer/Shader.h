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
	~Shader();

	static Shader ReadShader(const std::string& fileName, VkDevice device);

	VkShaderModule GetModule() const { return module; };

	ShaderType type;

private:
	uint32_t size;
	char* data;
	VkShaderModule module;

	void CreateModule(VkDevice device);
};

