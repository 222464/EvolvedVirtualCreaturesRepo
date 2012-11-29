#pragma once

class BatchRenderable
{
public:
	virtual ~BatchRenderable() {}
	virtual void BatchRender() = 0;
};

