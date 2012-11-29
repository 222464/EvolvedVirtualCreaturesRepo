#include <SceneEffects/Water/SceneEffect_WaterRender.h>

#include <Scene/Scene.h>

#include <SceneEffects/Light.h>

#include <assert.h>

SceneEffect_WaterRender::SceneEffect_WaterRender()
	: m_bumpMapOffset0(0.0f, 0.0f), m_bumpMapOffset1(0.0f, 0.0f), m_flowRate(0.001f)
{
}

SceneEffect_WaterRender::~SceneEffect_WaterRender()
{
	// Delete groupings
	for(std::list<WaterGeomGrouping*>::iterator it = m_pGeomGroupings.begin(); it != m_pGeomGroupings.end(); it++)
		delete *it;
}

bool SceneEffect_WaterRender::Create(SceneEffect_Lighting* pLighting, const std::string &m_waterShaderName, const std::string &bumpMap0Name, const std::string &bumpMap1Name)
{
	assert(GetScene() != NULL);

	m_pLighting = pLighting;

	if(!m_waterShader.LoadAsset(m_waterShaderName))
		return false;

	if(!m_bumpMap0.LoadAsset(bumpMap0Name))
		return false;

	if(!m_bumpMap1.LoadAsset(bumpMap1Name))
		return false;

	// Texture settings to loop
	m_bumpMap0.Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	m_bumpMap1.Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	Scene* pScene = GetScene();

	m_effectCopy.Create(pScene->m_gBuffer.GetWidth(), pScene->m_gBuffer.GetHeight(), false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	glBindTexture(GL_TEXTURE_2D, m_effectCopy.GetTextureID());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	// Defaults
	m_waterShader.Bind();

	m_waterShader.SetShaderTexture("gPosition", pScene->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	m_waterShader.SetShaderTexture("effectCopy", m_effectCopy.GetTextureID(), GL_TEXTURE_2D);

	m_waterShader.SetUniformv2f("effectSize", static_cast<float>(m_effectCopy.GetWidth()), static_cast<float>(m_effectCopy.GetHeight()));

	m_waterShader.SetUniformv3f("attenuation", m_pLighting->GetAttenuation());
	m_waterShader.SetUniformf("shininess", 10.0f);
	m_waterShader.SetUniformv4f("diffuseColor", Color4f(0.5f, 0.5f, 1.0f, 1.0f));
	m_waterShader.SetUniformf("specularColor", 1.0f);
	m_waterShader.SetUniformv3f("ambient", m_pLighting->m_ambient);

	m_waterShader.SetShaderTexture("bumpMap0", m_bumpMap0.GetTextureID(), GL_TEXTURE_2D);
	m_waterShader.SetShaderTexture("bumpMap1", m_bumpMap1.GetTextureID(), GL_TEXTURE_2D);

	m_waterShader.Unbind();

	return true;
}

void SceneEffect_WaterRender::RunEffect()
{
	Scene* pScene = GetScene();

	

	// Read from gBuffer, copy to temp buffer
	m_effectCopy.Bind_Draw();

	glBlitFramebuffer(0, 0, m_effectCopy.GetWidth(), m_effectCopy.GetHeight(), 0, 0, m_effectCopy.GetWidth(), m_effectCopy.GetHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	// Read from temp buffer
	m_effectCopy.Bind_Read();

	// Revert to drawing to gBuffer
	pScene->m_gBuffer.Bind_Draw();

	m_waterShader.Bind();
	m_waterShader.BindShaderTextures();

	Shader::Validate(m_waterShader.GetProgID());

	// Set offsets
	m_waterShader.SetUniformv2f("bumpMapOffset0", m_bumpMapOffset0);
	m_waterShader.SetUniformv2f("bumpMapOffset1", m_bumpMapOffset1);

	// ----------------------------- Lights set up -----------------------------

	// Containers for accumulating array uniform elements
	std::vector<int> types(8, Light::e_point);
	std::vector<float> positions(24, 0.0f);
	std::vector<float> colors(24, 0.0f);
	std::vector<float> spotDirections(24, 0.0f);
	std::vector<float> spotExponents(8, 0.5f);
	std::vector<float> spreadAngleCos(8, 0.5f);
	std::vector<float> ranges(8, 0.5f);
	std::vector<float> intensities(8, 0.5f);

	// Render all of them (added in order, since the SceneObject::Render function is called after sort by depth)
	for(std::list<WaterGeomGrouping*>::iterator it = m_pGeomGroupings.begin(); it != m_pGeomGroupings.end(); it++)
	{
		WaterGeomGrouping* pGrouping = *it;

		std::vector<Light*> regionLights;

		m_pLighting->GetLightsInArea(pGrouping->m_aabb, regionLights);

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

				if(lightsInPass > 0)
				{
					// Set uniform arrays in shader
					m_waterShader.SetUniform1iv("lightType", types.size(), &types[0]);
					m_waterShader.SetUniform3fv("lightPosition", positions.size() / 3, &positions[0]);
					m_waterShader.SetUniform3fv("lightColor", colors.size() / 3, &colors[0]);
					m_waterShader.SetUniform3fv("lightSpotDirection", spotDirections.size() / 3, &spotDirections[0]);
					m_waterShader.SetUniform1fv("lightSpotExponent", spotExponents.size(), &spotExponents[0]);
					m_waterShader.SetUniform1fv("lightSpreadAngleCos", spreadAngleCos.size(), &spreadAngleCos[0]);
					m_waterShader.SetUniform1fv("lightRange", ranges.size(), &ranges[0]);
					m_waterShader.SetUniform1fv("lightIntensity", intensities.size(), &intensities[0]);

					m_waterShader.SetUniformi("numLights", lightsInPass);

					// If on a second pass, turn off the ambient
					if(lightIndex >= 8)
						m_waterShader.SetUniformv3f("ambient", Vec3f(0.0f, 0.0f, 0.0f));
					else
						m_waterShader.SetUniformv3f("ambient", m_pLighting->m_ambient);
				}

				// Render geometry
				pGrouping->RenderGeomGrouping();
			}
		}

		if(regionLights.empty())
		{
			m_waterShader.SetUniformi("numLights", 0);

			// Get lights effecting the scene object. If more than 8, use multiple passes
			pGrouping->RenderGeomGrouping();
		}
	}

	m_waterShader.Unbind();

	// Revert to reading from gBuffer
	pScene->m_gBuffer.Bind_Read();

	// ---------------- Change with time ---------------

	// Move bump map offsets
	Vec2f offset(Vec2f(m_flowRate, m_flowRate) * pScene->m_frameTimer.GetTimeMultiplier());
	
	m_bumpMapOffset0 += offset;
	m_bumpMapOffset1 -= offset;
}

void SceneEffect_WaterRender::AddGrouping(WaterGeomGrouping* pGrouping)
{
	assert(GetScene() != NULL);

	m_pGeomGroupings.push_back(pGrouping);

	pGrouping->m_pScene = GetScene();
}