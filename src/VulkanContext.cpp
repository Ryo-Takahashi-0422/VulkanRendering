#include "VulkanContext.h"

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
/// インスタンス取得
/// </summary>
/// <returns></returns>
VulkanContext& VulkanContext::GetInstance() {
	static VulkanContext instance;
	return instance;
}