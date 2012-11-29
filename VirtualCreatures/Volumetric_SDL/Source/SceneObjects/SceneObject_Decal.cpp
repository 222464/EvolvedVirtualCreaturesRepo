#include <SceneObjects/SceneObject_Decal.h>

#include <Utilities/UtilFuncs.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Decal::SceneObject_Decal()
	: m_pDecalTexture_diffuse(NULL), m_age(0.0f), m_pProp(NULL), m_pDecalTexture_specular(NULL), m_pDecalTexture_normal(NULL),
	m_specularColor(0.0f)
{
}

bool SceneObject_Decal::Create(const std::string &fileName, const Vec3f &position, const Vec3f &direction, float despawnTime, float spriteWidth)
{
	assert(m_pDecalTexture_diffuse == NULL);
	assert(GetScene() != NULL);

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset(fileName, pAsset))
		return false;

	m_pDecalTexture_diffuse = static_cast<Asset_Texture*>(pAsset);

	m_transform = Matrix4x4f::TranslateMatrix(position) * Matrix4x4f::DirectionMatrix_AutoUp(direction);

	m_despawnTime = despawnTime;

	m_halfWidth = spriteWidth / 2.0f;
	m_halfHeight = m_halfWidth * (static_cast<float>(m_pDecalTexture_diffuse->GetWidth()) / m_pDecalTexture_diffuse->GetHeight());

	m_pDecalRenderer = static_cast<SceneObject_Decal_BatchRenderer*>(GetScene()->GetBatchRenderer("decal", SceneObject_Decal_BatchRenderer::SceneObject_Decal_BatchRendererFactory));

	return true;
}

bool SceneObject_Decal::Create(const std::string &fileName, SceneObject_Prop_Physics_Dynamic* pProp, const Vec3f &position, const Vec3f &direction, float despawnTime, float spriteWidth)
{
	assert(m_pDecalTexture_diffuse == NULL);
	assert(GetScene() != NULL);

	m_pProp = pProp;
	m_propTracker.Set(m_pProp);

	// Get a relative transform
	m_transform = m_pProp->GetInverseTransform() * Matrix4x4f::TranslateMatrix(position) * Matrix4x4f::DirectionMatrix_AutoUp(direction);

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset(fileName, pAsset))
		return false;

	m_pDecalTexture_diffuse = static_cast<Asset_Texture*>(pAsset);

	m_despawnTime = despawnTime;

	m_halfWidth = spriteWidth / 2.0f;
	m_halfHeight = m_halfWidth * (static_cast<float>(m_pDecalTexture_diffuse->GetWidth()) / m_pDecalTexture_diffuse->GetHeight());

	m_pDecalRenderer = static_cast<SceneObject_Decal_BatchRenderer*>(GetScene()->GetBatchRenderer("decal", SceneObject_Decal_BatchRenderer::SceneObject_Decal_BatchRendererFactory));

	return true;
}

bool SceneObject_Decal::AddSpecularMap(const std::string &fileName)
{
	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset(fileName, pAsset))
		return false;

	m_pDecalTexture_specular = static_cast<Asset_Texture*>(pAsset);

	return true;
}

bool SceneObject_Decal::AddNormalMap(const std::string &fileName)
{
	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset(fileName, pAsset))
		return false;

	m_pDecalTexture_normal = static_cast<Asset_Texture*>(pAsset);

	return true;
}

void SceneObject_Decal::Logic()
{
	m_age += GetScene()->m_frameTimer.GetTimeMultiplier();

	if(m_age > m_despawnTime)
		Destroy();

	if(m_pProp != NULL && !m_propTracker.ReferenceAlive())
		Destroy();
}

void SceneObject_Decal::Render_Batch_NoTexture()
{
	if(m_pProp == NULL)
		GetScene()->SetWorldMatrix(m_transform);
	else
		GetScene()->SetWorldMatrix(m_pProp->GetTransform() * m_transform);

	glBegin(GL_QUADS);
	glVertex3f(0.0f, m_halfHeight, -m_halfWidth);
	glVertex3f(0.0f, m_halfHeight, m_halfWidth);
	glVertex3f(0.0f, -m_halfHeight, m_halfWidth);
	glVertex3f(0.0f, -m_halfHeight, -m_halfWidth);
	glEnd();
}

void SceneObject_Decal::Render_Batch_Textured()
{
	if(m_pProp == NULL)
		GetScene()->SetWorldMatrix(m_transform);
	else
		GetScene()->SetWorldMatrix(m_pProp->GetTransform() * m_transform);

	if(m_pDecalTexture_normal != NULL)
	{
		GetScene()->SetCurrentGBufferRenderShader(Scene::e_bump);

		glActiveTexture(GL_TEXTURE3);

		m_pDecalTexture_normal->Bind();
	}
	else
		GetScene()->SetCurrentGBufferRenderShader(Scene::e_plain);

	if(m_pDecalTexture_specular != NULL)
	{
		GetScene()->UseSpecularTexture(true);

		glActiveTexture(GL_TEXTURE1);

		m_pDecalTexture_specular->Bind();
	}
	else
		GetScene()->UseSpecularTexture(false);

	glActiveTexture(GL_TEXTURE0);

	m_pDecalTexture_diffuse->Bind();

	if(m_specularColor != 0.0f)
	{
		GetScene()->SetSpecularColor(m_specularColor);

		glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2i(0, 0); glVertex3f(0.0f, m_halfHeight, -m_halfWidth);
		glTexCoord2i(1, 0); glVertex3f(0.0f, m_halfHeight, m_halfWidth);
		glTexCoord2i(1, 1); glVertex3f(0.0f, -m_halfHeight, m_halfWidth);
		glTexCoord2i(0, 1); glVertex3f(0.0f, -m_halfHeight, -m_halfWidth);
		glEnd();

		GetScene()->SetSpecularColor(0.0f);
	}
	else
	{
		glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2i(0, 0); glVertex3f(0.0f, m_halfHeight, -m_halfWidth);
		glTexCoord2i(1, 0); glVertex3f(0.0f, m_halfHeight, m_halfWidth);
		glTexCoord2i(1, 1); glVertex3f(0.0f, -m_halfHeight, m_halfWidth);
		glTexCoord2i(0, 1); glVertex3f(0.0f, -m_halfHeight, -m_halfWidth);
		glEnd();
	}
}

void SceneObject_Decal::Render()
{
	m_pDecalRenderer->Add(this);
}

SceneObject_Decal_BatchRenderer::SceneObject_Decal_BatchRenderer()
{
	// Every bit alone for entire byte
	m_decalIndices[0] = 0x01;
	m_decalIndices[1] = 0x02;
	m_decalIndices[2] = 0x04;
	m_decalIndices[3] = 0x08;
	m_decalIndices[4] = 0x10;
	m_decalIndices[5] = 0x20;
	m_decalIndices[6] = 0x40;
	m_decalIndices[7] = 0x80;
}

void SceneObject_Decal_BatchRenderer::Add(SceneObject_Decal* pDecal)
{
	m_pDecals.push_back(pDecal);
}

// Inherited from BatchRenderer
void SceneObject_Decal_BatchRenderer::Execute()
{
	if(!GetScene()->m_renderingDeferred)
		return;

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	
	glDepthMask(false);

	glEnable(GL_POLYGON_OFFSET_FILL);
	
	for(unsigned int i = 0, numDecals = m_pDecals.size(); i < numDecals;)
	{
		// Stencil in batches of 8
		glColorMask(false, false, false, false);
		Shader::Unbind();

		glPolygonOffset(2.0f, 2.0f);

		// Mark parts where depth test fails
		glStencilFunc(GL_ALWAYS, 0xff, 0xff);
		glStencilOp(GL_KEEP, GL_REPLACE, GL_KEEP);

		unsigned int firstDecalInBatchIndex = i;

		// Stencil out regions with stencil mask
		for(unsigned int j = 0; j < 8 && i < numDecals; j++, i++)
		{
			glStencilMask(m_decalIndices[j]);
			m_pDecals[i]->Render_Batch_NoTexture();
		}

		glColorMask(true, true, true, true);

		// Stencil test
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
		glPolygonOffset(-2.0f, -2.0f);

		// Re-render, second pass
		i = firstDecalInBatchIndex;

		GetScene()->RebindGBufferRenderShader();

		for(unsigned int j = 0; j < 8 && i < numDecals; j++, i++)
		{
			glStencilFunc(GL_EQUAL, 0xff, m_decalIndices[j]);

			m_pDecals[i]->Render_Batch_Textured();
		}

		glClear(GL_STENCIL_BUFFER_BIT);
	}

	GetScene()->SetCurrentGBufferRenderShader(Scene::e_plain);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(true);

	glDepthFunc(GL_LESS);

	glDisable(GL_STENCIL_TEST);
}

void SceneObject_Decal_BatchRenderer::Clear()
{
	m_pDecals.clear();
}

BatchRenderer* SceneObject_Decal_BatchRenderer::SceneObject_Decal_BatchRendererFactory()
{
	return new SceneObject_Decal_BatchRenderer();
}