#pragma once
#include "core/VulkanContext.h"

namespace Loader
{
    VkShaderModule LoadShaderModule(const 	std::filesystem::path& shaderSpvPath);
};