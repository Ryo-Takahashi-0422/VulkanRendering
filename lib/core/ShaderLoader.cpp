#include "pch.h"
#include "core/ShaderLoader.h"

namespace Loader
{
	VkShaderModule LoadShaderModule(const 	std::filesystem::path& shaderSpvPath) 
	{
		// ファイルをバイナリで読み込み
		std::ifstream file(shaderSpvPath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open shader file: " + shaderSpvPath.string());
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0).read(buffer.data(), fileSize);
		file.close();

		// VkShaderModuleCreateInfoにバイナリデータを設定
		VkShaderModuleCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = buffer.size(),
			.pCode = reinterpret_cast<const uint32_t*>(buffer.data())
		};

		VkDevice device = VulkanContext::GetInstance().GetVkDevice();
		VkShaderModule shaderModule{};
		auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module from: " + shaderSpvPath.string());
		}

		return shaderModule;
	}
}