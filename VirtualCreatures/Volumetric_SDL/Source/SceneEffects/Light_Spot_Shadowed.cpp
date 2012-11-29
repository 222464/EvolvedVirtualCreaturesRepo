#include <SceneEffects/Light_Spot_Shadowed.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Utilities/UtilFuncs.h>

#include <assert.h>

Light_Spot_Shadowed::Light_Spot_Shadowed(unsigned int shadowMap_width, unsigned int shadowMap_height)
	: m_shadowMap(shadowMap_width, shadowMap_height, true, GL_RGB16F, GL_RGB, GL_FLOAT), m_blurPingPong(shadowMap_width, shadowMap_height, true, GL_RGB16F, GL_RGB, GL_FLOAT)
{
	// Set up filtering
	/*glBindTexture(GL_TEXTURE_2D, m_shadowMap.GetTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);*/
}

Light_Spot_Shadowed::~Light_Spot_Shadowed()
{
}

void Light_Spot_Shadowed::SetShader(Scene* pScene)
{
	m_pLighting->m_spotLightShadowedEffectShader.Bind();
	m_pLighting->m_spotLightShadowedEffectShader.SetShaderTexture("shadowMap", m_shadowMap.GetTextureID(), GL_TEXTURE_2D);
	m_pLighting->m_spotLightShadowedEffectShader.BindShaderTextures();
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformmat4("lightBiasViewProjection", m_lightTransform * pScene->GetInverseViewMatrix());

	m_pLighting->m_spotLightShadowedEffectShader.SetUniformv3f("lightPosition", pScene->GetViewMatrix() * m_center);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformv3f("lightColor", m_color);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformf("lightRange", m_range);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformv3f("lightSpotDirection", pScene->GetNormalMatrix() * m_direction);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformf("lightSpotExponent", m_lightSpotExponent);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformf("lightSpreadAngleCos", m_spreadAngleCos);
	m_pLighting->m_spotLightShadowedEffectShader.SetUniformf("lightIntensity", m_intensity);
}

void Light_Spot_Shadowed::RenderToMap(Scene* pScene)
{
	pScene->m_enableShaderSwitches = false;

	// Bind the depth map FBO
	m_shadowMap.Bind();
	m_shadowMap.SetViewport();

	// ------------------------------ Render Depth Map ------------------------------

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(RadToDeg(m_spreadAngle * 2.0f), 1.0, 0.1f, 2000.0f);
	glMatrixMode(GL_MODELVIEW);

	float rangeOneMore = m_range + 1.0f;

	glClearColor(rangeOneMore, rangeOneMore * rangeOneMore, rangeOneMore, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Matrix4x4f viewMatrix(Matrix4x4f::CameraDirectionMatrix(m_direction, Vec3f(0.0f, 1.0f, 0.0f)) * Matrix4x4f::TranslateMatrix(-m_center));

	pScene->SetCustomViewMatrix(viewMatrix);

	pScene->ExtractFrustum(Matrix4x4f::GL_Get_Projection() * viewMatrix);

	pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());

	m_lightTransform = GetBias() * Matrix4x4f::GL_Get_Projection() * viewMatrix;

	//std::cout << GetBias() * Matrix4x4f::GL_Get_Projection() * viewMatrix * Vec3f(16.0f, 16.0f, 16.0f) << std::endl;

	// Bind shadow map rendering shader
	m_pLighting->m_spotLightStoreMomentsShader.Bind();
	m_pLighting->m_spotLightStoreMomentsShader.BindShaderTextures();

	// Change cull face to avoid artifacts
	//glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);

	// Render scene to the light distance
	pScene->Render_Distance(m_range);

	// Revert to normal cull face
	//glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	Shader::Unbind();

	FBO::Unbind();

	m_blurPingPong.Bind();

	// Ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Doesn't use m_zNear and m_zFar since ortho projections never really need anything adjustable
	glOrtho(0, m_shadowMap.GetWidth(), 0, m_shadowMap.GetHeight(), -1, 1); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// ------------------------------ Blur the Shadow Map ------------------------------

	glDisable(GL_DEPTH_TEST);

	m_pLighting->m_VSMblurShader_horizontal.Bind();
	m_pLighting->m_VSMblurShader_horizontal.SetShaderTexture("scene", m_shadowMap.GetTextureID(), GL_TEXTURE_2D);
	m_pLighting->m_VSMblurShader_horizontal.BindShaderTextures();
	m_pLighting->m_VSMblurShader_horizontal.SetUniformf("blurSize", 0.003f); 

	float widthf = static_cast<float>(m_shadowMap.GetWidth());
	float heightf = static_cast<float>(m_shadowMap.GetHeight());
	
	// Fullscreen quad to blur
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex3f(widthf, 0.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex3f(widthf, heightf, 0.0f);
	glTexCoord2i(0, 1); glVertex3f(0.0f, heightf, 0.0f);
	glEnd();

	

	FBO::Unbind();
	m_shadowMap.Bind();

	m_pLighting->m_VSMblurShader_vertical.Bind();
	m_pLighting->m_VSMblurShader_vertical.SetShaderTexture("scene", m_blurPingPong.GetTextureID(), GL_TEXTURE_2D);
	m_pLighting->m_VSMblurShader_vertical.BindShaderTextures();
	m_pLighting->m_VSMblurShader_vertical.SetUniformf("blurSize", 0.003f); 
	
	// Fullscreen quad to blur
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex3f(widthf, 0.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex3f(widthf, heightf, 0.0f);
	glTexCoord2i(0, 1); glVertex3f(0.0f, heightf, 0.0f);
	glEnd();

	glEnable(GL_DEPTH_TEST);
	
	

	// ------------------------------ Reset ------------------------------

	Shader::Unbind();

	pScene->m_pWin->SetViewport();
	pScene->m_pWin->SetProjection();

	

	// Unbind depth map FBO
	FBO::Unbind();

	pScene->m_enableShaderSwitches = true;

	GL_ERROR_CHECK();
}

unsigned int Light_Spot_Shadowed::GetColorTextureID()
{
	return m_shadowMap.GetTextureID();
}

Light::LightType Light_Spot_Shadowed::GetType()
{
	return e_spot_shadowed;
}