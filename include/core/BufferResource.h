#pragma once
#include "core/VulkanContext.h"
#include "core/GpuResourceBase.h"

class IBufferResource {
public:
	virtual bool IsHostAccessible() const = 0;
	virtual VkBuffer GetVkBuffer() const = 0;
	virtual VkDeviceSize GetBufferSize() const = 0;
	virtual void SetAccessFlags(const VkAccessFlags flags) = 0;
	virtual VkAccessFlags GetAccessFlags() const = 0;
	virtual void* Map() = 0;
	virtual void Unmap() = 0;
	virtual VkDescriptorBufferInfo GetDescriptorInfo() const = 0;
};

// Map(), Unmap()実装がないためこれ自体はインスタンス化不可能
template<typename T>
class BufferResource : public GpuResourceBase<T>, public IBufferResource
{
public:
	// コピーコンストラクタとコピー代入演算子禁止して
	// Vulkan のリソースハンドルなどを不用意にコピーし、破棄処理が重複することを防ぐ
	BufferResource(const BufferResource&) = delete;
	BufferResource& operator=(const BufferResource&) = delete;

	virtual ~BufferResource() { Cleanup(); }
	virtual void Cleanup();

	bool IsHostAccessible() const override { return (m_memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
	VkAccessFlags GetAccessFlags() const override { return m_accessFlags; }
	void SetAccessFlags(const VkAccessFlags flags) override { m_accessFlags = flags; }

	VkBuffer GetVkBuffer() const override { return m_buffer; }
	VkDeviceSize GetBufferSize() const override { return m_size; }

	VkDescriptorBufferInfo GetDescriptorInfo() const override;

protected:
	// 共通の基盤としては使うが、基底クラスそのものを外部から直接生成させない
	BufferResource() = default;
	bool CreateBuffer(const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memProps);
	VkBuffer m_buffer{};
	VkDeviceMemory m_memory{};
	VkDeviceSize m_size{};
	VkMemoryPropertyFlags m_memProps{};
	VkAccessFlags m_accessFlags = VK_ACCESS_NONE;
};

template<typename  T>
void BufferResource<T>::Cleanup()
{
	VulkanContext& context = VulkanContext::GetInstance();
	VkDevice device = context.GetVkDevice();

	if (m_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}

	m_size = 0;
}

template<typename  T>
bool BufferResource<T>::CreateBuffer(const VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags memProps)
{
	VulkanContext& context = VulkanContext::GetInstance();
	VkDevice device = context.GetVkDevice();

	auto result = vkCreateBuffer(device, &createInfo, nullptr, &m_buffer);
	if (result != VK_SUCCESS) {
		return false;
	}

	// メモリ要件を取得
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = context.FindMemoryType(memRequirements, memProps)
	};

	result = vkAllocateMemory(device, &allocInfo, nullptr, &m_memory);
	if (result != VK_SUCCESS) {
		return false;
	}

	vkBindBufferMemory(device, m_buffer, m_memory, 0);
	m_size = createInfo.size;
	m_memProps = memProps;

	return true;
}

template<typename T>
VkDescriptorBufferInfo BufferResource<T>::GetDescriptorInfo() const
{
	return VkDescriptorBufferInfo{
		.buffer = m_buffer,
		.offset = 0,
		.range = m_size
	};
}

class VertexBuffer : public BufferResource<VertexBuffer>
{
	friend class GpuResourceBase<VertexBuffer>;
private:
	VertexBuffer() = default;
public:
	virtual ~VertexBuffer() = default;
	virtual void* Map() override;
	virtual void Unmap() override;

	bool Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps);

	// create, Initializeを一度で処理するための作成関数
	static std::shared_ptr<VertexBuffer> Create(VkDeviceSize size, VkMemoryPropertyFlags memProps)
	{
		auto buffer = GpuResourceBase::Create();
		if (!buffer->Initialize(size, memProps)) { return nullptr; }
		return buffer;
	}

};

