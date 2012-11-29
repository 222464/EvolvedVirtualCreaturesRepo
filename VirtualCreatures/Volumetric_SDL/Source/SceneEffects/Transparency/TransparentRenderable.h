#pragma once

#include <Scene/SceneObject.h>

#define TRANSPARENT_RENDER_NAME "transRen"

class TransparentRenderable :
	public SceneObject
{
public:
	class SceneEffect_TransparencyRender* m_pTransparencyRender;

private:
	// Inherited from SceneObject
	void Render(); // Adds to the transparent render list
	void OnAdd(); // Adds this scene object to the transparent renderer scene effect

public:
	TransparentRenderable();
	virtual ~TransparentRenderable() {}

	// OnAdd / Render equivalents
	virtual void OnAdd_Transparent();
	virtual void Render_Transparent() = 0;
};