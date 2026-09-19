#pragma once
#include "ISampleApp.h"

class TriangleApp : public ISampleApp
{
public:
	virtual void OnInitialize() = 0;
	virtual void OnDrawFrame() = 0;
	virtual void OnCleanup() = 0;
};