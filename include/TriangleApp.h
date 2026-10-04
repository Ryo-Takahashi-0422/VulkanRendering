#pragma once
#include "core/VulkanContext.h"
#include "ISampleApp.h"

class TriangleApp : public ISampleApp
{
public:
	virtual void OnInitialize() override;
	virtual void OnDrawFrame() override;
	virtual void OnCleanup() override;
};