#pragma once

#include <Renderer/Octree/OctreeOccupant.h>

#include <Renderer/Shader/UBOShaderInterface.h>

#include <Scene/Scene.h>

class Light :
	public OctreeOccupant
{
public:
	enum LightType
	{
		e_point = 0, e_spot, e_point_shadowed, e_spot_shadowed, e_directional, e_directional_shadowed
	};

protected:
	class SceneEffect_Lighting* m_pLighting;
	
public:
	bool m_enabled;

	Light();
	virtual ~Light() {}

	virtual void SetTransform(Scene* pScene) = 0;
	virtual void SetShader(Scene* pScene) = 0;
	virtual void RenderBoundingGeom() = 0;
	virtual bool Intersects(const Vec3f &position) = 0;
	virtual LightType GetType() = 0;

	friend class SceneEffect_Lighting;
};