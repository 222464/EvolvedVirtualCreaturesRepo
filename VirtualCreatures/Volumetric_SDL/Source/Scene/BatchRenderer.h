#pragma once

#include <Scene/BatchRenderable.h>

class BatchRenderer
{
private:
	class Scene* m_pScene;

public:
	virtual ~BatchRenderer() {}
	virtual void Execute() = 0;

	// Must remove all rendering tasks
	virtual void Clear() = 0;

	class Scene* GetScene()
	{
		return m_pScene;
	}

	friend class Scene;
};

