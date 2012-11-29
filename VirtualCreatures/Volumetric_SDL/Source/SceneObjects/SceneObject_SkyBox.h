#pragma once

#include <Scene/SceneObject.h>

#include <Renderer/Octree.h>
#include <Renderer/Model_OBJ_VertexOnly.h>

class SceneObject_SkyBox :
	public SceneObject
{
private:
	Asset_Texture m_texFront;
	Asset_Texture m_texBack;
	Asset_Texture m_texLeft;
	Asset_Texture m_texRight;
	Asset_Texture m_texTop;
public:
	float m_radius;
	float m_brightness;

	SceneObject_SkyBox();

	bool Create(const std::string &textureRootName, const std::string &extension);

	// Inherited from SceneObject
	void Render();
};