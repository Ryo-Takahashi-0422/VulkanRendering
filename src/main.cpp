#include "pch.h"
#include <GlfwDisplayWindow.h>
#include <TriangleApp.h>
#include <Windows.h>
#include <VulkanContext.h>
#include <GLFWSurfaceProvider.h>

int __stdcall wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	// 初期化設定
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// ウィンドウの作成
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Triangle", nullptr, nullptr);
	GLFWSurfaceProvider surfaceProvider(window);

	// Vulkanの初期化
	auto& vulkanCtx = VulkanContext::GetInstance();
	vulkanCtx.GetWindowSystemExtensions = [=](auto& extensionList) {
		uint32_t extCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);
		if (extCount > 0) {
			extensionList.insert(extensionList.end(), extensions, extensions + extCount);
		}
	};
	vulkanCtx.Initialize("Triangle", &surfaceProvider);

	// スワップチェイン初期化
	vulkanCtx.RecreateSwapchain();

	// アプリケーションの初期化
	TriangleApp theApp{};
	theApp.OnInitialize();

	// メッセージループ処理
	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		glfwPollEvents();

		// 描画処理
		theApp.OnDrawFrame();
	}

	// 終了処理
	theApp.OnCleanup();
	vulkanCtx.Cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}