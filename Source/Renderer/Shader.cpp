#include "Shader.h"
#include "Device.h"
#include "VulkanUtils.h"
#include <fstream>

Shader::Shader(const std::string& fileName, VkDevice device)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    size = (uint32_t)file.tellg();
    data = (char*)malloc((size_t)size);

    file.seekg(0);
    file.read(data, size);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data);

    VULKAN_CALL(vkCreateShaderModule(device, &createInfo, nullptr, &module));
}

Shader::~Shader()
{
    vkDestroyShaderModule(Device::Get()->GetVulkanDevice(), module, nullptr);
    free(data);
}
