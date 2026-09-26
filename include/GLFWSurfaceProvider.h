#pragma once
#include "pch.h"
#include "ISurfaceProvider.h"
#define GLFW_INCLUDE_VULKAN

class GLFWSurfaceProvider : public  ISurfaceProvider{
public:
	explicit GLFWSurfaceProvider(GLFWwindow* window);
	VkSurfaceKHR CreateSurface(VkInstance instance) override;
	uint32_t GetFramebufferWidth() const override;
	uint32_t GetFramebufferHeight() const override;

private:
	GLFWwindow* m_window;
};