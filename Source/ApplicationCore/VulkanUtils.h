#pragma once

#include "ErrorManager.h"

#include <vulkan/vulkan.h>

#define VULKAN_CALL(func) { VkResult result = func; if (result != VK_SUCCESS) { ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "_##func", "Vulkan", result); exit(0); } }
#define VULKAN_CALL_MSG(func, message) { VkResult result = func; if (result != VK_SUCCESS) { ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "_##func", "Vulkan", result, message); exit(0); } }