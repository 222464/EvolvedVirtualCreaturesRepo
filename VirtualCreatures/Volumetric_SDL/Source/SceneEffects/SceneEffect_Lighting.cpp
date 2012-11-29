#include <SceneEffects/SceneEffect_Lighting.h>

#include <Scene/Scene.h>

#include <assert.h>

#include <Utilities/UtilFuncs.h>

#include <Renderer/RenderUtils.h>

#include <iostream>

SceneEffect_Lighting::SceneEffect_Lighting()
	: m_created(false), m_ambient(0.0f, 0.0f, 0.0f), m_attenuation(0.05f, 0.3f, 0.1f)
{
	// Every bit alone for entire byte
	m_lightIndices[0] = 0x01;
	m_lightIndices[1] = 0x02;
	m_lightIndices[2] = 0x04;
	m_lightIndices[3] = 0x08;
	m_lightIndices[4] = 0x10;
	m_lightIndices[5] = 0x20;
	m_lightIndices[6] = 0x40;
	m_lightIndices[7] = 0x80;

	// Inverted
	/*m_lightIndices[0] = 0xfe;
	m_lightIndices[1] = 0xfd;
	m_lightIndices[2] = 0xfb;
	m_lightIndices[3] = 0xf7;
	m_lightIndices[4] = 0xef;
	m_lightIndices[5] = 0xdf;
	m_lightIndices[6] = 0xbf;
	m_lightIndices[7] = 0x7f;*/
}

bool SceneEffect_Lighting::Create(const std::string &pointLightEffectShaderName,
		const std::string &spotLightEffectShaderName, const std::string &spotLightShadowedEffectShaderName, const std::string &spotLightStoreMomentsEffectShaderName,
		const std::string &VSMhorizontalBlurShaderName, const std::string &VSMverticalBlurShaderName,
		const std::string &directionalLightEffectShaderName, const std::string &directionalLightShadowedEffectShaderName, const std::string &directionalLightStoreMomentsEffectShaderName,
		const std::string &nullShaderName, const std::string &sphereModelName, const std::string &coneModelName)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	if(!m_pointLightEffectShader.LoadAsset(pointLightEffectShaderName))
		return false;

	if(!m_spotLightEffectShader.LoadAsset(spotLightEffectShaderName))
		return false;

	if(!m_spotLightShadowedEffectShader.LoadAsset(spotLightShadowedEffectShaderName))
		return false;

	if(!m_spotLightStoreMomentsShader.LoadAsset(spotLightStoreMomentsEffectShaderName))
		return false;

	if(!m_VSMblurShader_horizontal.LoadAsset(VSMhorizontalBlurShaderName))
		return false;

	if(!m_VSMblurShader_vertical.LoadAsset(VSMverticalBlurShaderName))
		return false;

	if(!m_directionalLightEffectShader.LoadAsset(directionalLightEffectShaderName))
		return false;

	if(!m_directionalLightShadowedEffectShader.LoadAsset(directionalLightShadowedEffectShaderName))
		return false;

	if(!m_directionalLightStoreMomentsShader.LoadAsset(directionalLightStoreMomentsEffectShaderName))
		return false;

	if(!m_nullShader.LoadAsset(nullShaderName))
		return false;

	if(!m_sphere.LoadAsset(sphereModelName))
		return false;

	if(!m_cone.LoadAsset(coneModelName))
		return false;

	// ---------------------------- Shader and SPT Setup ----------------------------

	m_lightSPT.Create(GetScene()->m_spt.GetRootAABB());

	// ---------------------------------------- Point Light Shader Setup ----------------------------------------

	const float shininess = 46.0f;

	m_pointLightEffectShader.Bind();

	m_pointLightEffectShader.SetShaderTexture("gColor", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_pointLightEffectShader.SetShaderTexture("gPosition", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_pointLightEffectShader.SetShaderTexture("gNormal", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);
	//m_pointLightEffectShader.SetShaderTexture("gDepth", GetScene()->m_gBuffer.GetDepthTextureID(), GL_TEXTURE);

	m_pointLightEffectShader.SetUniformv2f("gTexSize", static_cast<float>(GetScene()->m_gBuffer.GetWidth()), static_cast<float>(GetScene()->m_gBuffer.GetHeight()));
	m_pointLightEffectShader.SetUniformv3f("attenuation", m_attenuation);

	m_pointLightEffectShader.SetUniformf("shininess", shininess);

	// Attribute locations
	m_pointLightPositionLocation = m_pointLightEffectShader.GetAttributeLocation("lightPosition");
	m_pointLightColorLocation = m_pointLightEffectShader.GetAttributeLocation("lightColor");
	m_pointLightRangeLocation = m_pointLightEffectShader.GetAttributeLocation("lightRange");
	m_pointLightIntensityLocation = m_pointLightEffectShader.GetAttributeLocation("lightIntensity");

	// ---------------------------------------- Spot Light Shader Setup ----------------------------------------

	m_spotLightEffectShader.Bind();

	m_spotLightEffectShader.SetShaderTexture("gColor", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_spotLightEffectShader.SetShaderTexture("gPosition", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_spotLightEffectShader.SetShaderTexture("gNormal", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);

	m_spotLightEffectShader.SetUniformv2f("gTexSize", static_cast<float>(GetScene()->m_gBuffer.GetWidth()), static_cast<float>(GetScene()->m_gBuffer.GetHeight()));
	m_spotLightEffectShader.SetUniformv3f("attenuation", m_attenuation);

	m_spotLightEffectShader.SetUniformf("shininess", shininess);

	// ---------------------------------------- Spot Light Shadowed Shader Setup ----------------------------------------

	// Moment shader setup
	m_spotLightStoreMomentsShader.Bind();
	m_spotLightStoreMomentsShader.SetShaderTexture("diffuseMap", 0, GL_TEXTURE_2D);

	// Light shader setup
	m_spotLightShadowedEffectShader.Bind();

	m_spotLightShadowedEffectShader.SetShaderTexture("gColor", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_spotLightShadowedEffectShader.SetShaderTexture("gPosition", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_spotLightShadowedEffectShader.SetShaderTexture("gNormal", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);

	m_spotLightShadowedEffectShader.SetUniformv2f("gTexSize", static_cast<float>(GetScene()->m_gBuffer.GetWidth()), static_cast<float>(GetScene()->m_gBuffer.GetHeight()));
	m_spotLightShadowedEffectShader.SetUniformv3f("attenuation", m_attenuation);

	m_spotLightShadowedEffectShader.SetUniformf("shininess", shininess);

	// ---------------------------------------- Directional Light Shader Setup ----------------------------------------

	// Moment shader setup
	m_directionalLightEffectShader.Bind();
	
	m_directionalLightEffectShader.SetShaderTexture("gColor", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_directionalLightEffectShader.SetShaderTexture("gPosition", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_directionalLightEffectShader.SetShaderTexture("gNormal", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);

	m_directionalLightEffectShader.SetUniformv2f("gTexSize", static_cast<float>(GetScene()->m_gBuffer.GetWidth()), static_cast<float>(GetScene()->m_gBuffer.GetHeight()));

	m_directionalLightEffectShader.SetUniformf("shininess", shininess);

	// ---------------------------------------- Directional Light Shadowed Shader Setup ----------------------------------------

	// Moment shader setup
	m_directionalLightShadowedEffectShader.Bind();
	
	m_directionalLightShadowedEffectShader.SetShaderTexture("gColor", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_directionalLightShadowedEffectShader.SetShaderTexture("gPosition", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_directionalLightShadowedEffectShader.SetShaderTexture("gNormal", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);

	m_directionalLightShadowedEffectShader.SetUniformv2f("gTexSize", static_cast<float>(GetScene()->m_gBuffer.GetWidth()), static_cast<float>(GetScene()->m_gBuffer.GetHeight()));

	m_directionalLightShadowedEffectShader.SetUniformf("shininess", shininess);

	Shader::Unbind();

	GL_ERROR_CHECK();

	m_created = true;

	return true;
}

SceneEffect_Lighting::~SceneEffect_Lighting()
{
	ClearLights();
}

void SceneEffect_Lighting::AddLight(Light* pLight)
{
	m_lights.insert(pLight);

	m_lightSPT.Add(pLight);

	pLight->m_pLighting = this;
}

void SceneEffect_Lighting::RemoveLight(Light* pLight)
{
	std::unordered_set<Light*>::iterator it = m_lights.find(pLight);

	if(it == m_lights.end())
	{
		std::cerr << "Attempted to remove an unregistered light!" << std::endl;
		return;
	}

	pLight->RemoveFromTree();

	delete pLight;

	m_lights.erase(it);
}

void SceneEffect_Lighting::AddLight_Directional(Light_Directional* pLight)
{
	m_directionalLights.insert(pLight);

	pLight->m_pLighting = this;
}

void SceneEffect_Lighting::RemoveLight_Directional(Light_Directional* pLight)
{
	std::unordered_set<Light_Directional*>::iterator it = m_directionalLights.find(pLight);

	if(it == m_directionalLights.end())
	{
		std::cerr << "Attempted to remove an unregistered light!" << std::endl;
		return;
	}

	delete pLight;

	m_directionalLights.erase(it);
}

void SceneEffect_Lighting::ClearLights()
{
	for(std::unordered_set<Light*>::iterator it = m_lights.begin(); it != m_lights.end(); it++)
		delete *it;

	for(std::unordered_set<Light_Directional*>::iterator it = m_directionalLights.begin(); it != m_directionalLights.end(); it++)
		delete *it;

	m_lights.clear();
	m_directionalLights.clear();
}

void SceneEffect_Lighting::RunEffect()
{
	assert(m_created);
	
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glColor3f(m_ambient.r, m_ambient.g, m_ambient.b);
	
	Scene* pScene = GetScene();

	pScene->m_gBuffer.DrawAsQuad(GBuffer::e_color, pScene);

	glColor3f(1.0f, 1.0f, 1.0f);
	
	// ---------------------------- Shader Setup ----------------------------

	// All have same textures, so just set one of them
	m_pointLightEffectShader.BindShaderTextures();

	// ---------------------------- Render Lights ----------------------------

	// Directional lights - full screen quad
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for(std::unordered_set<Light_Directional*>::iterator it = m_directionalLights.begin(); it != m_directionalLights.end(); it++)
	{
		(*it)->SetShader(pScene);
		DrawNormalizedQuad();
	}

	pScene->m_pWin->SetProjection();

	glEnable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);

	// Query visible lights
	std::vector<OctreeOccupant*> result;

	m_lightSPT.Query_Frustum(result, pScene->GetFrustum());

	glEnable(GL_VERTEX_ARRAY);

	glEnable(GL_STENCIL_TEST);

	glClearStencil(0);

	for(unsigned int i = 0, size = result.size(); i < size;)
	{
		glClear(GL_STENCIL_BUFFER_BIT);

		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glColorMask(false, false, false, false);

		// Batch 8 lights together
		unsigned int firstLightIndex = i;

		glStencilFunc(GL_ALWAYS, 0xff, 0xff);

		for(unsigned int j = 0; j < 8 && i < size; j++, i++)
		{
			glStencilMask(m_lightIndices[j]);

			Light* pLight = static_cast<Light*>(result[i]);

			if(!pLight->m_enabled)
				continue;

			pLight->SetTransform(pScene);
			pLight->RenderBoundingGeom();
		}

		i = firstLightIndex;

		glColorMask(true, true, true, true);

		// Now render with reversed depth testing and only to stenciled regions
		glCullFace(GL_FRONT);

		glDepthFunc(GL_GREATER);

		glEnable(GL_BLEND);

		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		for(unsigned int j = 0; j < 8 && i < size; j++, i++)
		{
			glStencilFunc(GL_EQUAL, 0xff, m_lightIndices[j]);

			Light* pLight = static_cast<Light*>(result[i]);

			if(!pLight->m_enabled)
				continue;

			// If camera is inside light, do not perform depth test (would cull it away improperly)
			if(pLight->Intersects(pScene->m_camera.m_position))
			{
				glDisable(GL_STENCIL_TEST);

				pLight->SetTransform(pScene);

				pLight->SetShader(pScene);

				pLight->RenderBoundingGeom();

				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				pLight->SetTransform(pScene);

				pLight->SetShader(pScene);

				pLight->RenderBoundingGeom();
			}
		}

		glCullFace(GL_BACK);

		glDepthFunc(GL_LESS);

		glDisable(GL_BLEND);

		Shader::Unbind();
	}

	// Re-enable stencil writes to all bits
	glStencilMask(0xff);

	glDisable(GL_VERTEX_ARRAY);

	glDisable(GL_STENCIL_TEST);

	GL_ERROR_CHECK();
}

void SceneEffect_Lighting::BindPointLightShader()
{
	m_pointLightEffectShader.Bind();
}

void SceneEffect_Lighting::BindSpotLightShader()
{
	m_spotLightEffectShader.Bind();
}

void SceneEffect_Lighting::SetPointLightShininess(float shininess)
{
	assert(m_created);
	
	m_pointLightEffectShader.SetUniformf("shininess", shininess);
}

void SceneEffect_Lighting::SetSpotLightShininess(float shininess)
{
	assert(m_created);
	
	m_spotLightEffectShader.SetUniformf("shininess", shininess);
}

void SceneEffect_Lighting::RenderSphere()
{
	m_sphere.Render();
}

void SceneEffect_Lighting::RenderCone()
{
	m_cone.Render();
}

void SceneEffect_Lighting::SetAttenuation(const Vec3f &attenuation)
{
	m_attenuation = attenuation;

	// Update shader attuenuation uniforms
	// ---------------------------------------- Point Light Shader Setup ----------------------------------------

	m_pointLightEffectShader.Bind();

	m_pointLightEffectShader.SetUniformv3f("attenuation", m_attenuation);

	// ---------------------------------------- Spot Light Shader Setup ----------------------------------------

	m_spotLightEffectShader.Bind();

	m_spotLightEffectShader.SetUniformv3f("attenuation", m_attenuation);

	// ---------------------------------------- Spot Light Shadowed Shader Setup ----------------------------------------

	// Light shader setup
	m_spotLightShadowedEffectShader.Bind();

	m_spotLightShadowedEffectShader.SetUniformv3f("attenuation", m_attenuation);

	Shader::Unbind();
}

const Vec3f &SceneEffect_Lighting::GetAttenuation() const
{
	return m_attenuation;
}

void SceneEffect_Lighting::GetLightsInArea(const AABB &aabb, std::vector<Light*> &lights) const
{
	std::vector<OctreeOccupant*> octreeOccupants;

	m_lightSPT.Query_Region(aabb, octreeOccupants);

	for(unsigned int i = 0, size = octreeOccupants.size(); i < size; i++)
		lights.push_back(static_cast<Light*>(octreeOccupants[i]));
}

Matrix4x4f GetBias()
{
	Matrix4x4f mat;

	mat.m_elements[0] = 0.5f; mat.m_elements[4] = 0.0f; mat.m_elements[8] = 0.0f; mat.m_elements[12] = 0.5f;
	mat.m_elements[1] = 0.0f; mat.m_elements[5] = 0.5f; mat.m_elements[9] = 0.0f; mat.m_elements[13] = 0.5f;
	mat.m_elements[2] = 0.0f; mat.m_elements[6] = 0.0f; mat.m_elements[10] = 0.5f; mat.m_elements[14] = 0.5f;
	mat.m_elements[3] = 0.0f; mat.m_elements[7] = 0.0f; mat.m_elements[11] = 0.0f; mat.m_elements[15] = 1.0f;

	return mat;
}
