#include <SceneEffects/Transparency/SceneEffect_TransparencyRender.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <algorithm>

#include <assert.h>

SceneEffect_TransparencyRender::SceneEffect_TransparencyRender()
	: m_pLighting(NULL), m_rendering(false)
{
}

bool SceneEffect_TransparencyRender::Create(SceneEffect_Lighting* pLighting, const std::string &forwardLightingName)
{
	assert(m_pLighting == NULL);

	m_pLighting = pLighting;

	if(!m_forwardLightingShader.LoadAsset(forwardLightingName))
		return false;
	
	// Defaults
	m_forwardLightingShader.Bind();

	m_forwardLightingShader.SetUniformv3f("attenuation", m_pLighting->GetAttenuation());
	m_forwardLightingShader.SetUniformf("shininess", 10.0f);
	m_forwardLightingShader.SetUniformv4f("diffuseColor", 1.0f, 1.0f, 1.0f, 1.0f);
	m_forwardLightingShader.SetUniformf("specularColor", 1.0f);
	m_forwardLightingShader.SetUniformv3f("ambient", m_pLighting->m_ambient);

	m_forwardLightingShader.SetShaderTexture("diffuseMap", GetScene()->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // TEXTURE0
	m_forwardLightingShader.SetShaderTexture("specularMap", GetScene()->GetWhiteTexture().GetTextureID(), GL_TEXTURE_2D); // TEXTURE1

	m_forwardLightingShader.Unbind();

	GL_ERROR_CHECK();

	return true;
}

void SceneEffect_TransparencyRender::RunEffect()
{
	if(m_pTransparencyRenderables.empty())
		return;
	
	Scene* pScene = GetScene();

	// OpenGL settings for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind lighting shader
	m_forwardLightingShader.Bind();
	m_forwardLightingShader.BindShaderTextures();

	m_rendering = true;

	// Containers for accumulating array uniform elements
	std::vector<int> types(8, Light::e_point);
	std::vector<float> positions(24, 0.0f);
	std::vector<float> colors(24, 0.0f);
	std::vector<float> spotDirections(24, 0.0f);
	std::vector<float> spotExponents(8, 0.5f);
	std::vector<float> spreadAngleCos(8, 0.5f);
	std::vector<float> ranges(8, 0.5f);
	std::vector<float> intensities(8, 0.5f);

	std::sort(m_pTransparencyRenderables.begin(), m_pTransparencyRenderables.end(), &Scene::DistCompare);

	// Render all of them (added in order, since the SceneObject::Render function is called after sort by depth)
	for(unsigned int i = 0, size = m_pTransparencyRenderables.size(); i < size; i++)
	{
		TransparentRenderable* pRenderable = m_pTransparencyRenderables[i];

		std::vector<Light*> regionLights;

		m_pLighting->GetLightsInArea(pRenderable->GetAABB(), regionLights);

		for(unsigned int lightIndex = 0, numLights = regionLights.size(); lightIndex < numLights;)
		{
			unsigned int lightsInPass = 0;

			// Start a pass, set up shader
			for(; lightsInPass < 8 && lightIndex < numLights; lightsInPass++, lightIndex++)
			{
				Light* pLight = regionLights[lightIndex];

				types[lightsInPass] = pLight->GetType();

				switch(pLight->GetType())
				{
				case Light::e_point:
					{
						Light_Point* pPoint = static_cast<Light_Point*>(pLight);

						// Set temp uniform arrays
						SetVec3f(positions, lightsInPass, pScene->GetViewMatrix() * pPoint->GetCenter());
						SetColor3f(colors, lightsInPass, pPoint->m_color);
					
						ranges[lightsInPass] = pPoint->GetRange();
						intensities[lightsInPass] = pPoint->GetIntensity();
					}

					break;
				case Light::e_spot_shadowed: // Treat shadowed spot light like normal spot light in this case
				case Light::e_spot:
					{
						Light_Spot* pSpot = static_cast<Light_Spot*>(pLight);

						// Set temp uniform arrays
						SetVec3f(positions, lightsInPass, pScene->GetViewMatrix() * pSpot->GetCenter());
						SetColor3f(colors, lightsInPass, pSpot->m_color);

						ranges[lightsInPass] = pSpot->GetRange();
						intensities[lightsInPass] = pSpot->GetIntensity();

						SetVec3f(spotDirections, lightsInPass, pScene->GetNormalMatrix() * pSpot->GetDirection());
						spotExponents[lightsInPass] = pSpot->m_lightSpotExponent;
						spreadAngleCos[lightsInPass] = pSpot->GetSpreadAngleCos();
					}

					break;
				}
			}

			if(lightsInPass == 0)
				continue;

			// Set uniform arrays in shader
			m_forwardLightingShader.SetUniform1iv("lightType", types.size(), &types[0]);
			m_forwardLightingShader.SetUniform3fv("lightPosition", positions.size() / 3, &positions[0]);
			m_forwardLightingShader.SetUniform3fv("lightColor", colors.size() / 3, &colors[0]);
			m_forwardLightingShader.SetUniform3fv("lightSpotDirection", spotDirections.size() / 3, &spotDirections[0]);
			m_forwardLightingShader.SetUniform1fv("lightSpotExponent", spotExponents.size(), &spotExponents[0]);
			m_forwardLightingShader.SetUniform1fv("lightSpreadAngleCos", spreadAngleCos.size(), &spreadAngleCos[0]);
			m_forwardLightingShader.SetUniform1fv("lightRange", ranges.size(), &ranges[0]);
			m_forwardLightingShader.SetUniform1fv("lightIntensity", intensities.size(), &intensities[0]);

			m_forwardLightingShader.SetUniformi("numLights", lightsInPass);

			// If on a second pass, turn off the ambient
			if(lightIndex >= 8)
				m_forwardLightingShader.SetUniformv3f("ambient", Vec3f(0.0f, 0.0f, 0.0f));
			else
				m_forwardLightingShader.SetUniformv3f("ambient", m_pLighting->m_ambient);

			// Render geometry
			pRenderable->Render_Transparent();
		}

		if(regionLights.empty())
		{
			m_forwardLightingShader.SetUniformi("numLights", 0);

			// Get lights effecting the scene object. If more than 8, use multiple passes
			pRenderable->Render_Transparent();
		}
	}

	m_rendering = false;

	m_forwardLightingShader.Unbind();

	// Clear list for next frame
	m_pTransparencyRenderables.clear();

	// Reset OpenGL states
	glDisable(GL_BLEND);
}

void SceneEffect_TransparencyRender::SetDiffuseColor(const Color3f &color)
{
	assert(m_rendering);

	m_forwardLightingShader.SetUniformv3f("diffuseColor", color);
}

void SceneEffect_TransparencyRender::SetSpecularColor(const Color3f &color)
{
	assert(m_rendering);

	m_forwardLightingShader.SetUniformv3f("specularColor", color);
}