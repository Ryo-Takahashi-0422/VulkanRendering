#include "pch.h"
#include "TriangleApp.h"
#include "core/Swapchain.h"
#include "core/ImageLayoutTransition.h"
#include "core/ShaderLoader.h"
#include "core/AssetPath.h"

void TriangleApp::OnInitialize()
{
	InitializeTriangleVertexBuffer();
	InitializeGraphicsPipeline();
}

void TriangleApp::InitializeTriangleVertexBuffer()
{
	const std::vector<Vertex> triangleVertices = {
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
	};

	VkDeviceSize bufferSize = sizeof(Vertex) * triangleVertices.size();
	m_vertexBuffer = VertexBuffer::Create(bufferSize,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* p = m_vertexBuffer->Map();
	memcpy(p, triangleVertices.data(), bufferSize);
	m_vertexBuffer->Unmap();
}

void TriangleApp::InitializeGraphicsPipeline()
{
	VkShaderModule vertShaderModule = Loader::LoadShaderModule(GetAssetPath(AssetType::Shader, "triangle/triangle.vert.spv"));
	VkShaderModule fragShaderModule = Loader::LoadShaderModule(GetAssetPath(AssetType::Shader, "triangle/triangle.frag.spv"));

	VkPipelineShaderStageCreateInfo shaderStageVS{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	VkPipelineShaderStageCreateInfo shaderStageFS{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = { shaderStageVS ,shaderStageFS, };
}

void TriangleApp::OnDrawFrame() {
	auto& vulkanCtx = VulkanContext::GetInstance();
	auto& swapChain = vulkanCtx.GetSwapChain();
	auto vkDevice = vulkanCtx.GetVkDevice();

	// ウィンドウ最小化対策　TODO:要検討
	if (vulkanCtx.AcuireNextImage() != VK_SUCCESS) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return;
	}

	auto* frameCtx = vulkanCtx.GetCurrentFrameContext();
	auto& commandBuffer = frameCtx->commandBuffer;
	commandBuffer->Begin();

	// 描画前：UNDEFINED → COLOR_ATTACHMENT_OPTIONAL
	// VK_ATTACHMENT_LOAD_OP_CLEARを指定のため、常にUNDEFINED指定とする
	VkImageSubresourceRange range{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0, .levelCount = 1,
		.baseArrayLayer = 0, .layerCount = 1,
	};
	commandBuffer->TransitionLayout(
		swapChain->GetCurrentImage(), range, ImageLayoutTransition::FromUnderfinedToColorAttachment()
	);

	auto imageView = swapChain->GetCurrentView();
	auto extent = swapChain->GetExtent();

	VkRenderingAttachmentInfo colorAttachment{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = imageView,
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = VkClearValue{.color = {0.5f, 0.2f, 0.3f, 1.0f} },
	};
	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = { {0, 0}, extent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
	};
	vkCmdBeginRendering(*commandBuffer, &renderingInfo);

	vkCmdEndRendering(*commandBuffer);

	// 表示用レイアウト変更
	commandBuffer->TransitionLayout(
		swapChain->GetCurrentImage(), range, ImageLayoutTransition::FromColorToPresent()
	);

	commandBuffer->End();

	vulkanCtx.SubmitPresent();
}

void TriangleApp::OnCleanup()
{
	auto& vulkanCtx = VulkanContext::GetInstance();
	auto vkDevice = vulkanCtx.GetVkDevice();

	// GPUがアイドル状態になってから後始末の開始
	vkDeviceWaitIdle(vkDevice);

	if (m_pipeliine != VK_NULL_HANDLE) {
		vkDestroyPipeline(vkDevice, m_pipeliine, nullptr);
	}

	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(vkDevice, m_pipelineLayout, nullptr);
	}

	m_vertexBuffer->Cleanup();
	m_vertexBuffer.reset();
}