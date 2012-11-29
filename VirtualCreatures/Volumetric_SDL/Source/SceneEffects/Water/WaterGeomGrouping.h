#pragma once

#define WATER_RENDER_NAME "waterRen"

#include <Constructs/AABB.h>

#include <Scene/Scene.h>

class WaterGeomGrouping
{
private:
	Scene* m_pScene;

protected:
	AABB m_aabb;

public:
	virtual ~WaterGeomGrouping() {}

	// OnAdd / Render equivalents
	virtual void RenderGeomGrouping() = 0;

	Scene* GetScene() const
	{
		return m_pScene;
	}

	friend class SceneEffect_WaterRender;
};