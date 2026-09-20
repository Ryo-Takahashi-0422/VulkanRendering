#include "pch.h"
#include "VulkanContext.h"


void VulkanContext::Initialize(const char* appName, ISurfaceProvider* surfaceProvider) {
	m_surfaceProvider = surfaceProvider;
	CreateInstance(appName); // Vulkanデバイスの作成
	PickPhysicalDevice();	 // 物理デバイスの選択
	CreateDebugMessenger();  // デバッグ機能について準備
	CreateLogicalDevice();   // 論理デバイスの選択
	CreateCommandPool();     // コマンドプールの作成
	CreateDescriptorPool();  // ディスクリプタプールの作成
}

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

	// ....

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

VulkanContext& VulkanContext::Get() {
	static VulkanContext instance;
	return instance;
}