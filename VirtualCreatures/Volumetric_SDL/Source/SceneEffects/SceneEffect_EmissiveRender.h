#pragma once

#include <Scene/SceneEffect.h>

#include <vector>

class SceneEffect_EmissiveRender :
	public SceneEffect
{
public:
	struct EmissiveRenderable
	{
		virtual void Render() = 0;
	};

private:
	std::vector<EmissiveRenderable*> m_pEmissiveRenderables;

public:
	// Cleared at end of frame. Not deleted though, users must manage themselves
	void AddForFrame(EmissiveRenderable* pRenderable);

	// Inherited from SceneEffect
	void RunEffect();
};

