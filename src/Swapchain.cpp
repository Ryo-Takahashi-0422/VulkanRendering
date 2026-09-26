#include "Swapchain.h"

bool Swapchain::Recreate(uint32_t newWidth, uint32_t newHeight) {
	auto& vulkanCtx = VulkanContext::GetInstance();
	auto vkPhysicalDevice = vulkanCtx.GetVkPhysicalDevice();
	auto vkDevice = vulkanCtx.GetVkDevice();
	auto surface = vulkanCtx.GetSurface();

	// surfaceのキャパシティを取得
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface, &caps);
	VkExtent2D extent = caps.currentExtent;
	if (extent.width == UINT32_MAX) {
		extent.width = newWidth;
		extent.height = newHeight;
	}

	// サーフェスのフォーマットを取得
	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &count, formats.data());

	// 出力フォーマットの選択
	VkSurfaceFormatKHR format = formats[0];
	for (auto& surfaceFormat : formats) {
		if (surfaceFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			continue;
		}

		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM ||
			surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
			format = surfaceFormat;
			break;
		}
	}

	auto imageCount = std::max(3u, caps.minImageCount);

	// スワップチェイン作成
	VkSwapchainCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = caps.minImageCount + 1;
	info.imageFormat = format.format;
	info.imageColorSpace = format.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	info.clipped = VK_TRUE;
	info.oldSwapchain = m_swapChain;

	// GPUがアイドル状態になってからスワップチェインの再生成
	vkDeviceWaitIdle(vkDevice);

	VkSwapchainKHR swapchain{};
	if (vkCreateSwapchainKHR(vkDevice, &info, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain");
	}

	// イメージの取得
	m_swapChain = swapchain;
	m_imageFormat = format;
	m_imageExtent = extent;

	vkGetSwapchainImagesKHR(vkDevice, m_swapChain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(vkDevice, m_swapChain, &imageCount, m_images.data());

	for (uint32_t i = 0; i < m_images.size(); ++i) {
		VkImageViewCreateInfo imageViewCI{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format.format,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY, // R
				VK_COMPONENT_SWIZZLE_IDENTITY, // G
				VK_COMPONENT_SWIZZLE_IDENTITY, // B
				VK_COMPONENT_SWIZZLE_IDENTITY, // A
			},
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		VkImageView view;
		vkCreateImageView(vkDevice, &imageViewCI, nullptr, &view);
		m_imageViews.push_back(view);
	}
	CreateFrameContext();
	return true;
}

/// <summary>
/// フレームの初期化
/// </summary>
void Swapchain::CreateFrameContext() {
	auto& vulkanCtx = VulkanContext::GetInstance();
	auto vkDevice = vulkanCtx.GetVkDevice();
	m_frames.resize(m_images.size());
	for (auto& frame : m_frames) {
		VkSemaphoreCreateInfo semCI{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		vkCreateSemaphore(vkDevice, &semCI, nullptr, &frame.renderComplete);
	}

	uint32_t presentCompleteSemaphoreCount = m_images.size() + 1;
	m_presentSemaphoreList.reserve(presentCompleteSemaphoreCount);
	for (uint32_t i = 0; i < presentCompleteSemaphoreCount; ++i) {
		VkSemaphoreCreateInfo semCI{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VkSemaphore semaphore;
		vkCreateSemaphore(vkDevice, &semCI, nullptr, &semaphore);
		m_presentSemaphoreList.push_back(semaphore);
	}
}

/// <summary>
/// フレームの破棄
/// </summary>
void Swapchain::DestroyFrameContext() {
	auto& vulkanCtx = VulkanContext::GetInstance();
	auto vkDevice = vulkanCtx.GetVkDevice();
	for (auto& frame : m_frames) {
		vkDestroySemaphore(vkDevice, frame.presentComplete, nullptr);
		vkDestroySemaphore(vkDevice, frame.renderComplete, nullptr);
	}
	m_frames.clear();
	for (auto& sem : m_presentSemaphoreList) {
		vkDestroySemaphore(vkDevice, sem, nullptr);
	}
	m_presentSemaphoreList.clear();
}