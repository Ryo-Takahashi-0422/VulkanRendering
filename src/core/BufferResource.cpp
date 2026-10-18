#include "pch.h"
#include "core/BufferResource.h"

bool VertexBuffer::Initialize(VkDeviceSize size, VkMemoryPropertyFlags memProps)
{
	VulkanContext& context = VulkanContext::GetInstance();
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	SetAccessFlags(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	return CreateBuffer(bufferInfo, memProps);
}

void* VertexBuffer::Map() 
{
	if (!(m_memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return nullptr;

	void* mapped = nullptr;
	vkMapMemory(VulkanContext::GetInstance().GetVkDevice(), m_memory, 0, m_size, 0, &mapped);
	return mapped;
}

void VertexBuffer::Unmap()
{
	if (!(m_memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return;
	vkUnmapMemory(VulkanContext::GetInstance().GetVkDevice(), m_memory);
}