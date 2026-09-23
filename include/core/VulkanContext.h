#pragma once
#include "pch.h"

#include "core/CommandBuffer.h"

class Swapchain;
class CommandBuffer;
class ISurfaceProvider;

class VulkanContext 
{
public:
	static constexpr uint32_t MaxInflightFrames = 2;
	static VulkanContext& GetInstance();

	// 初期化
	void Initialize(const char* appName, ISurfaceProvider* surfaceProvider);

	// 終了処理
	void Cleanup();

	// スワップチェインの作成
	void RecreateSwapchain();
	
	// 各種Vulkanオブジェクト取得
	VkInstance getVkInstance() const { return m_vkInstance; };
	VkDevice GetVkDevice() const { return m_vkDevice; };
	VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; };
	VkDescriptorPool GetVkDescriptorPool() const { return m_descriptorPool; };
	VkQueue GetGraphicsQueue() const { return m_graphicsQueue; };
	uint32_t GetGraphicsFamily() const { return m_graphicsQueueFamilyIndex; };
	uint32_t GetPresentFamily() const { return m_presentQueueFamilyIndex; };
	VkCommandPool GetCommandPool() const { return m_commandPool; };
	VkSurfaceKHR GetSurface() const { return m_surface; };

	// コマンドバッファの作成
	std::shared_ptr<CommandBuffer> CreateCommandBuffer();

	// ディスクリプタセットの確保
	VkDescriptorSet AllcateDescriptorSet(VkDescriptorSetLayout layout);
	// ディスクリプタセットの解放
	void FreeDescriptorSet(VkDescriptorSet descriptorSet);

	// 描画フレーム単位で取り扱うコンテキスト情報
	struct FrameContext
	{
		std::shared_ptr<CommandBuffer> commandBuffer;
		VkFence inflightFence = VK_NULL_HANDLE;
	};
	// 現在のフレームインデックスを取得
	uint32_t GetCurrentFrameIndex() const { return m_currentFrameIndex; };
	// 描画可能なスワップチェーンイメージの切り替え
	VkResult AcuireNextImage();

	// 現在のフレームコンテキストのコマンドを実行し、プレゼンテーションを実行
	void SubmitPresent();

	// 指定されたコマンドバッファを実行し、完了を待機
	void SubmitAndWait(std::shared_ptr<CommandBuffer> commandBuffer);

	// 現在フレームコンテキストの取得
	FrameContext* GetCurrentFrameContext();

	//スワップチェインの取得
	std::unique_ptr<Swapchain>& GetSwapChain() { return m_swapChain; };

	//メモリタイプの取得
	uint32_t FindMemoryType(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags properties) const;

	// Function Callback(s)
	std::function<void(std::vector<const char*>&)> GetWindowSystemExtensions;

	// オブジェクトにデバッグ用の名前を設定する
	void SetDebugObjectName(void* objectHandle, VkObjectType type, const char* name);

private:
	VulkanContext() = default;
	~VulkanContext() = default;
private:
	void CreateInstance(const char* appName);
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateDebugMessenger();
	void CreateCommandPool();
	void CreateSycronizer();
	void CreateDescriptorPool();
	void CreateFrameContexts();
	void DestroyFrameContexts();

	void AdvanceFrame();
	void BuildVkFeatures();

	// Vulkanの構造体pNextを繋ぐ処理簡略化のためのテンプレート
	template<typename T>
	void BuildVkExtensionChain(T& last) {
		last.pNext = nullptr;
	}
	template<typename T, typename U, typename ... Rest>
	void BuildVkExtensionChain(T& current, U& next, Rest& ... rest) {
		current.pNext = next;
		BuildVkExtensionChain(next, rest ...);
	}

	ISurfaceProvider* m_surfaceProvider{};
	VkInstance m_vkInstance{};
	VkDevice m_vkDevice{};
	VkPhysicalDevice m_vkPhysicalDevice{};
	VkDescriptorPool m_descriptorPool{};
	VkQueue m_graphicsQueue{};
	uint32_t m_graphicsQueueFamilyIndex{};
	uint32_t m_presentQueueFamilyIndex{};
	VkCommandPool m_commandPool{};
	VkFence m_fence{};
	VkSemaphore m_semaphore{};
	VkSurfaceKHR m_surface{};
	VkPhysicalDeviceMemoryProperties m_memoryProperties{};
	VkPhysicalDeviceProperties m_physicalDeviceProperties{};

	std::vector<FrameContext> m_frameContext;
	std::unique_ptr<Swapchain> m_swapChain;

	VkDebugUtilsMessengerEXT m_debugMessenger{};
	PFN_vkSetDebugUtilsObjectNameEXT m_pfnSetDebugUtilsObjectNameEXT{};

	uint32_t m_currentFrameIndex = 0;

	// ------
	VkPhysicalDeviceFeatures2 m_physDevFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
	};
	VkPhysicalDeviceVulkan11Features m_vulkan11Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
	};

	VkPhysicalDeviceVulkan12Features m_vulkan12Features{
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
	};
	VkPhysicalDeviceVulkan13Features m_vulkan13Features{
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
	};
	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT m_atomicFloatFeatures{
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT
	};
};