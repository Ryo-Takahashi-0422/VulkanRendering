#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <functional>
#include <stdint.h>
#include <string>
#include <cstring>

#include "core/command_buffer.h"

class VulkanContext 
{
public:
	static constexpr uint32_t MaxInflightFrames = 2;
	static VulkanContext& Get();

	// ‰Šú‰»
	void Initialize(const char* appName, ISurfaceProvider* surfaceProvider);

	// I—¹ˆ—
	void Cleanup();

	// Function Callback(s)
	std::function<void(std::vector<const char*>&)> GetWindowSystemExtensions;

private:
	VulkanContext() = default;
	~VulkanContext() = default;

	ISurfaceProvider* m_surfaceProvider{};
};