#pragma once
#include "core/VulkanContext.h"
#include "ISampleApp.h"
#include <glm/glm.hpp>
#include <BufferResource.h>

class TriangleApp : public ISampleApp
{
public:
	virtual void OnInitialize() override;
	virtual void OnDrawFrame() override;
	virtual void OnCleanup() override;

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	void InitializeTriangleVertexBuffer();
	void InitializeGraphicsPipeline();

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	VkPipeline m_pipeliine = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};