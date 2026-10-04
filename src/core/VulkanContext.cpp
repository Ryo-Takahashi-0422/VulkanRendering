#include "pch.h"
#include "VulkanContext.h"
#include "ISurfaceProvider.h"
#include "Swapchain.h"

/// <summary>
/// 初期化
/// </summary>
/// <param name="appName"></param>
/// <param name="surfaceProvider"></param>
void VulkanContext::Initialize(const char* appName, ISurfaceProvider* surfaceProvider) {
	m_surfaceProvider = surfaceProvider;
	CreateInstance(appName); // Vulkanデバイスの作成
	PickPhysicalDevice();	 // 物理デバイスの選択
	CreateDebugMessenger();  // デバッグ機能について準備
	CreateLogicalDevice();   // 論理デバイスの選択
	CreateCommandPool();     // コマンドプールの作成
	CreateDescriptorPool();  // ディスクリプタプールの作成
	CreateSycronizer(); // フェンス、セマフォの作成
}

void VulkanContext::RecreateSwapchain() {
	if (m_swapChain == nullptr) {
		m_swapChain = std::make_unique<Swapchain>();
	}

	if (m_surface == VK_NULL_HANDLE) {
		CreateSurface();
	}

	auto width = m_surfaceProvider->GetFramebufferWidth();
	auto height = m_surfaceProvider->GetFramebufferHeight();
	m_swapChain->Recreate(width, height);

	DestroyFrameContexts();
	CreateFrameContexts();
}

/// <summary>
/// インスタンス生成
/// </summary>
/// <param name="appName"></param>
void VulkanContext::CreateInstance(const char* appName) {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VulkanBookEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	std::vector<const char*> extensionList;
	std::vector<const char*> layerList;

#if DEBUG || _DEBUG
	// 開発時には検証レイヤーを有効化する
	// 検証レイヤーから表示されるオブジェクトの名前を指定するための機能利用
	extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	layerList.push_back("VK_LAYER_KHRONOS_validation");
#endif

	// GLFWから有効化する拡張機能名を回収
	GetWindowSystemExtensions(extensionList);

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = uint32_t(extensionList.size());
	createInfo.ppEnabledExtensionNames = extensionList.data();
	createInfo.enabledLayerCount = uint32_t(layerList.size());;
	createInfo.ppEnabledLayerNames = layerList.data();

	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance");
}

/// <summary>
/// 物理デバイス取得
/// </summary>
void VulkanContext::PickPhysicalDevice() {
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &count, nullptr);
	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(m_vkInstance, &count, devices.data());
	m_vkPhysicalDevice = devices[0];

	// 情報取得
	vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &m_memoryProperties);
	vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_physicalDeviceProperties);
}

/// <summary>
/// 論理デバイスの作成
/// </summary>
void VulkanContext::CreateLogicalDevice() {
	//グラフィックスのキューインデックスを調査
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueCount, nullptr);
	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueCount, queues.data());

	m_graphicsQueueFamilyIndex = ~0u;
	for (uint32_t i = 0; const auto& props : queues) {
		if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			m_graphicsQueueFamilyIndex = i;
			break;
		}
		i++;
	}

	// 拡張機能の設定
	BuildVkFeatures();
	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	float priority = 1.0f;
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priority;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceInfo.pNext = &m_physDevFeatures;
	deviceInfo.pEnabledFeatures = nullptr;

	auto result = vkCreateDevice(m_vkPhysicalDevice, &deviceInfo, nullptr, &m_vkDevice);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logocal device");
	}

	vkGetDeviceQueue(m_vkDevice, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
}

/// <summary>
/// デバイス拡張機能の準備
/// </summary>
void VulkanContext::BuildVkFeatures() {
	BuildVkExtensionChain(
		m_physDevFeatures,
		m_vulkan11Features, m_vulkan12Features, m_vulkan13Features);
	// サポート状態を取得(Vulkan1.1以降の機能)
	vkGetPhysicalDeviceFeatures2(m_vkPhysicalDevice, &m_physDevFeatures);

	// サポート状況を確認
	if (!m_vulkan13Features.dynamicRendering)
	{
		// 非対応時の処理
	}
	m_vulkan13Features.dynamicRendering = VK_TRUE;

	if (!m_vulkan13Features.synchronization2)
	{
		// 非対応時の処理
	}
	m_vulkan13Features.synchronization2 = VK_TRUE;

}

/// <summary>
/// コマンドプールの作成
/// </summary>
void VulkanContext::CreateCommandPool() {
	VkCommandPoolCreateInfo commadPoolCI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	};

	commadPoolCI.queueFamilyIndex = m_graphicsQueueFamilyIndex;
	commadPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(m_vkDevice, &commadPoolCI, nullptr, &m_commandPool);
}

/// <summary>
/// デバッグコールバック
/// </summary>
/// <param name="severity"></param>
/// <param name="type"></param>
/// <param name="pCallbackData"></param>
/// <param name="pUserData"></param>
/// <returns></returns>
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	std::stringstream ss;
	ss << "[Validation Layer]" << pCallbackData->pMessage << std::endl;

#if defined(WIN32)
	OutputDebugStringA(ss.str().c_str());
#else
	std::cerr << ss.str();
#endif
	return VK_FALSE;
}

/// <summary>
/// デバッグ機能を有効化
/// </summary>
void VulkanContext::CreateDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = VulkanDebugCallback;

	auto vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT");

	if (vkCreateDebugUtilsMessenger && 
		vkCreateDebugUtilsMessenger(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger!");
	}

	m_pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(m_vkInstance, "vkSetDebugUtilsObjectNameEXT");
}

/// <summary>
/// オブジェクトにデバッグ用の名前を設定する
/// </summary>
/// <param name="objectHandle"></param>
/// <param name="type"></param>
/// <param name="name"></param>
void VulkanContext::SetDebugObjectName(void* objectHandle, VkObjectType type, const char* name) {
#if _DEBUG || DEBUG
	if (m_pfnSetDebugUtilsObjectNameEXT) {
		VkDebugUtilsObjectNameInfoEXT nameInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectType = type,
		.objectHandle = reinterpret_cast<uint64_t>(objectHandle),
		.pObjectName = name,
		};
		m_pfnSetDebugUtilsObjectNameEXT(m_vkDevice, &nameInfo);
	}
#endif
}

/// <summary>
/// 同期オブジェクトの作成
/// </summary>
void VulkanContext::CreateSycronizer() {

	// フェンスの作成
	VkFenceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};

	vkCreateFence(m_vkDevice, &createInfo, nullptr, &m_fence);

	// セマフォの作成
	VkSemaphoreCreateInfo semaphoreCI{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	vkCreateSemaphore(m_vkDevice, &semaphoreCI, nullptr, &m_semaphore);
}

/// <summary>
/// サーフェスの作成
/// </summary>
void VulkanContext::CreateSurface() {
	m_surface = m_surfaceProvider->CreateSurface(m_vkInstance);

	// グラフィックスキューはこのサーフェスへPresentを発行できるか
	VkBool32 present = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, m_graphicsQueueFamilyIndex, m_surface, &present);
	if (present == VK_FALSE) {
		throw std::runtime_error("not supported presentation");
	}
}

/// <summary>
/// コマンドバッファの作成
/// </summary>
/// <returns></returns>
std::shared_ptr<CommandBuffer> VulkanContext::CreateCommandBuffer() {
	VkCommandBufferAllocateInfo commandAI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer{};
	vkAllocateCommandBuffers(m_vkDevice, &commandAI, &commandBuffer);

	return std::make_shared<CommandBuffer>(commandBuffer);
}

/// <summary>
/// フレームコンテキスト(毎フレームに使用するデータ)の作成
/// </summary>
void VulkanContext::CreateFrameContexts() {
	m_frameContext.resize(MaxInflightFrames);
	for (auto& frame : m_frameContext) {
		frame.commandBuffer = CreateCommandBuffer();
		VkFenceCreateInfo fenceCI{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		vkCreateFence(m_vkDevice, &fenceCI, nullptr, &frame.inflightFence);
	}
}

/// <summary>
/// 描画可能なスワップチェーンイメージの切り替え
/// </summary>
/// <returns></returns>
VkResult VulkanContext::AcuireNextImage() {
	auto* frame = GetCurrentFrameContext();
	auto fence = frame->inflightFence;
	vkWaitForFences(m_vkDevice, 1, &fence, VK_TRUE, UINT64_MAX);
	
	auto result = m_swapChain->AcquireNextImage();
	if (result == VK_SUCCESS) {
		vkResetFences(m_vkDevice, 1, &fence);
	}
	else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// 最小化時の対策 0*0サイズの際に発生
		//auto width = m_surfaceProvider->GetFramebufferWidth();
		//auto height = m_surfaceProvider->GetFramebufferHeight();
		//while (width == 0 || height == 0) {
		//	// ウィンドウが復元されるまで待つ
		//}
	}
	assert(result != VK_ERROR_DEVICE_LOST); // デバイスロスト状態ならここで停止
	return result;
}

/// <summary>
/// 現在のフレームコンテキストのコマンドを実行し、プレゼンテーションを実行
/// 「Acquire → 描画 → Present」というGPU上の処理順序をSemaphoreで組み立て、
/// さらにFenceでCPUとGPUの進行を同期している
/// </summary>
void VulkanContext::SubmitPresent() {
	auto& frame = m_frameContext[GetCurrentFrameIndex()];

	// presentCompleteSemがシグナル状態になるまで、GPUのどの段階を待たせるか
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
	};

	// 本フレームで使用するセマフォを取得する
	VkSemaphore renderCompleteSem = m_swapChain->GetRenderCompleteSemaphore();
	VkSemaphore presentCompleteSem = m_swapChain->GetPresentCompleteSemaphore();

	// このフレームで記録したGPUコマンドを取得
	VkCommandBuffer commandBuffer = frame.commandBuffer->Get();

	// サブミット情報の構築
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	// presentCompleteSem→待つ→CommandBuffer実行
	// Swapchain Imageが使用可能になってから描画を開始するということ
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSem;
	// 描画終了時にSemaphoreをSignalする GPU描画完了→renderCompleteSem→Present可能
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSem;

	// GPUへ実行を依頼　vkWaitForFences()していたのはこのため
	// frame.inflightFenceにより、このフレームのGPU処理が全部終わったらこのFenceをSignalするよう設定
	auto result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, frame.inflightFence);
	assert(result != VK_ERROR_DEVICE_LOST); // デバイスロストなら停止

	// 描画したSwapchain Imageを画面に表示するよう要求
	// GraphicsQueueが既にpresentをサポートしていることはチェック済
	// vkQueuePresentKHR(...)呼び出し中→renderCompleteSemを待つことで
	// 描画完了→renderCompleteSem→Presentという順序を保証する
	m_swapChain->QueuePresent(m_graphicsQueue);
	AdvanceFrame();
}

void VulkanContext::AdvanceFrame() {
	m_currentFrameIndex = (m_currentFrameIndex + 1) % MaxInflightFrames;
}

void VulkanContext::CreateDescriptorPool() {
}

void VulkanContext::DestroyFrameContexts() {
	for (auto& frame : m_frameContext) {
		vkDestroyFence(m_vkDevice, frame.inflightFence, nullptr);
	}
}

void VulkanContext::Cleanup(){
	// デバイスがアイドル状態になってから破棄処理を進める
	vkDeviceWaitIdle(m_vkDevice);

	DestroyFrameContexts();
	vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);

	if (m_debugMessenger != VK_NULL_HANDLE)
	{
		auto func =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(
					m_vkInstance,
					"vkDestroyDebugUtilsMessengerEXT"
				)
				);

		if (func != nullptr)
		{
			func(
				m_vkInstance,
				m_debugMessenger,
				nullptr
			);
		}
		m_debugMessenger = VK_NULL_HANDLE;
	}

	if (m_swapChain) {
		m_swapChain->Cleanup();
		m_swapChain.reset();
	}

	if (m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
	}

	vkDestroyDevice(m_vkDevice, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
	m_vkDevice = VK_NULL_HANDLE;
	m_vkInstance = VK_NULL_HANDLE;
}

/// <summary>
/// インスタンス取得
/// </summary>
/// <returns></returns>
VulkanContext& VulkanContext::GetInstance() {
	static VulkanContext instance;
	return instance;
}