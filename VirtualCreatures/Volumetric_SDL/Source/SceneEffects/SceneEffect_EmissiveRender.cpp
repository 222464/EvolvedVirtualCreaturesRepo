#include <SceneEffects/SceneEffect_EmissiveRender.h>

#include <Renderer/SDL_OpenGL.h>

void SceneEffect_EmissiveRender::AddForFrame(EmissiveRenderable* pRenderable)
{
	m_pEmissiveRenderables.push_back(pRenderable);
}

void SceneEffect_EmissiveRender::RunEffect()
{
	// Write depth
	glDepthMask(true);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Run through all emissive
	for(unsigned int i = 0, size = m_pEmissiveRenderables.size(); i < size; i++)
		m_pEmissiveRenderables[i]->Render();

	// Reset
	glDepthMask(false);

	glDisable(GL_BLEND);

	// Clear for next frame
	m_pEmissiveRenderables.clear();
}