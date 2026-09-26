#include "GLFWSurfaceProvider.h"

GLFWSurfaceProvider::GLFWSurfaceProvider(GLFWwindow* window) {

}

/// <summary>
/// surface(コンポジターとVulkanのパイプオブジェクト)作成
/// </summary>
/// <param name="instance"></param>
/// <returns></returns>
VkSurfaceKHR GLFWSurfaceProvider::CreateSurface(VkInstance instance) {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create GLFW window surface.");
	}
	return surface;
}

/// <summary>
/// ウィンドウ幅取得
/// </summary>
/// <returns></returns>
uint32_t GLFWSurfaceProvider::GetFramebufferWidth() const {
	int width;
	glfwGetFramebufferSize(m_window, &width, nullptr);
	return static_cast<uint32_t>(width);
}

/// <summary>
/// ウィンドウ高さ取得
/// </summary>
/// <returns></returns>
uint32_t GLFWSurfaceProvider::GetFramebufferHeight() const {
	int Height;
	glfwGetFramebufferSize(m_window, nullptr, &Height );
	return static_cast<uint32_t>(Height);
}