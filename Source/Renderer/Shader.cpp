#include "Shader.h"
#include "Device.h"
#include <fstream>

Shader::~Shader()
{
    vkDestroyShaderModule(Device::Get()->GetVulkanDevice(), module, nullptr);
    free(data);
}

Shader Shader::ReadShader(const std::string& fileName, VkDevice device)
{
    Shader shader;

    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    shader.size = (uint32_t)file.tellg();
    shader.data = (char*)malloc((size_t)shader.size);

    file.seekg(0);
    file.read(shader.data, shader.size);
    file.close();

    shader.CreateModule(device);

    return shader;
}

void Shader::CreateModule(VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data);

    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS)
    {
        exit(0);
    }
}
