#include <SceneEffects/Transparency/TransparentRenderable.h>

#include <SceneEffects/Transparency/SceneEffect_TransparencyRender.h>

#include <Scene/Scene.h>

#include <assert.h>

TransparentRenderable::TransparentRenderable()
	: m_pTransparencyRender(NULL)
{
}

void TransparentRenderable::OnAdd()
{
	// Get transparency renderable if not already set (may be set be user as an optimization prior to this)
	if(m_pTransparencyRender == NULL)
	{
		m_pTransparencyRender = static_cast<SceneEffect_TransparencyRender*>(GetScene()->GetNamed_Effect(TRANSPARENT_RENDER_NAME));

		assert(m_pTransparencyRender != NULL);
	}

	OnAdd_Transparent();
}

void TransparentRenderable::Render()
{
	// Add self to transparency render list for the frame
	m_pTransparencyRender->m_pTransparencyRenderables.push_back(this);
}

void TransparentRenderable::OnAdd_Transparent()
{
}