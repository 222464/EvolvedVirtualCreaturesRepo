#include <SceneEffects/Light_Directional_Shadowed.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Constructs/Matrix4x4f.h>
#include <Constructs/Matrix3x3f.h>

#include <Utilities/UtilFuncs.h>

#include <Renderer/RenderUtils.h>

#include <string>
#include <sstream>

Light_Directional_Shadowed::Light_Directional_Shadowed(unsigned int shadowMapResolution, unsigned int cascadeResolutionDecreaseFactor, float maxDistance, float lambda, Scene* pScene)
{
	// Create shadow cascades
	float zCurrent = pScene->m_pWin->m_zNear;

	for(unsigned int i = 0; i < s_numCascades; i++)
	{
		float zNext = GetSplitDistance(i + 1, lambda, pScene->m_pWin->m_zNear, maxDistance);

		m_splitDistances[i] = zNext;

		// Create shadow maps
		m_shadowMaps[i].Create(shadowMapResolution, shadowMapResolution, true, GL_RGB16F, GL_RGB, GL_FLOAT);
		m_shadowMapBlurPingPongs[i].Create(shadowMapResolution, shadowMapResolution, false, GL_RGB16F, GL_RGB, GL_FLOAT);

		shadowMapResolution /= cascadeResolutionDecreaseFactor;

		zCurrent = zNext;
	}
}

void Light_Directional_Shadowed::UpdateProjection(Scene* pScene)
{
	Vec3f viewDirection(GetOrientationVector(pScene->GetViewMatrix()));
	Vec3f viewUp(GetUpVector(pScene->GetViewMatrix()));

	/*Matrix3x3f lightCoordinateSystem;

	// Assign basis vectors
	Vec3f basis_z(m_direction);
	Vec3f basis_x(basis_z.Cross(viewUp));
	Vec3f basis_y(basis_x.Cross(basis_z));

	basis_x.NormalizeThis();
	basis_y.NormalizeThis();
	basis_z.NormalizeThis();

	lightCoordinateSystem.m_elements[0] = basis_x.x;
	lightCoordinateSystem.m_elements[1] = basis_y.x;
	lightCoordinateSystem.m_elements[2] = basis_z.x;
	lightCoordinateSystem.m_elements[3] = basis_x.y;
	lightCoordinateSystem.m_elements[4] = basis_y.y;
	lightCoordinateSystem.m_elements[5] = basis_z.y;
	lightCoordinateSystem.m_elements[6] = basis_x.z;
	lightCoordinateSystem.m_elements[7] = basis_y.z;
	lightCoordinateSystem.m_elements[8] = basis_z.z;

	lightCoordinateSystem.m_elements[0] = basis_x.x;
	lightCoordinateSystem.m_elements[1] = basis_x.y;
	lightCoordinateSystem.m_elements[2] = basis_x.z;
	lightCoordinateSystem.m_elements[3] = basis_y.x;
	lightCoordinateSystem.m_elements[4] = basis_y.y;
	lightCoordinateSystem.m_elements[5] = basis_y.z;
	lightCoordinateSystem.m_elements[6] = basis_z.x;
	lightCoordinateSystem.m_elements[7] = basis_z.y;
	lightCoordinateSystem.m_elements[8] = basis_z.z;*/

	// Get frustum corner points
	Vec3f previousPoints[4];

	// Initial one
	GetFrustumCornerPoints(pScene, pScene->m_pWin->m_zNear, previousPoints);

	for(unsigned int i = 0; i < s_numCascades; i++)
	{
		float dist = m_splitDistances[i];

		Vec3f points[4];

		GetFrustumCornerPoints(pScene, dist, points);

		// Get average position for center of light camera
		Vec3f averagePosition(0.0f, 0.0f, 0.0f);

		for(unsigned int j = 0; j < 4; j++)
			averagePosition += previousPoints[j];

		for(unsigned int j = 0; j < 4; j++)
			averagePosition += points[j];

		averagePosition /= 8.0f;

		m_cascadeViews[i] = Matrix4x4f::CameraDirectionMatrix(m_direction, viewDirection) * Matrix4x4f::TranslateMatrix(-averagePosition); // View

		// Project and find bounds
		// First one is done manually
		AABB bounds;
		bounds.m_lowerBound = bounds.m_upperBound = m_cascadeViews[i] * points[0];

		for(unsigned int j = 1; j < 4; j++)
		{
			// Expand AABB
			Vec3f projectedPoint(m_cascadeViews[i] * points[j]);

			if(projectedPoint.x < bounds.m_lowerBound.x)
				bounds.m_lowerBound.x = projectedPoint.x;
			if(projectedPoint.y < bounds.m_lowerBound.y)
				bounds.m_lowerBound.y = projectedPoint.y;
			if(projectedPoint.z < bounds.m_lowerBound.z)
				bounds.m_lowerBound.z = projectedPoint.z;

			if(projectedPoint.x > bounds.m_upperBound.x)
				bounds.m_upperBound.x = projectedPoint.x;
			if(projectedPoint.y > bounds.m_upperBound.y)
				bounds.m_upperBound.y = projectedPoint.y;
			if(projectedPoint.z > bounds.m_upperBound.z)
				bounds.m_upperBound.z = projectedPoint.z;
		}

		// Go over previous points as well (TODO: DON'T RECALCULATE PROJECTION FOR PREVIOUS POINTS!)
		for(unsigned int j = 0; j < 4; j++)
		{
			Vec3f projectedPoint(m_cascadeViews[i] * previousPoints[j]);

			// Expand AABB
			if(projectedPoint.x < bounds.m_lowerBound.x)
				bounds.m_lowerBound.x = projectedPoint.x;
			if(projectedPoint.y < bounds.m_lowerBound.y)
				bounds.m_lowerBound.y = projectedPoint.y;
			if(projectedPoint.z < bounds.m_lowerBound.z)
				bounds.m_lowerBound.z = projectedPoint.z;

			if(projectedPoint.x > bounds.m_upperBound.x)
				bounds.m_upperBound.x = projectedPoint.x;
			if(projectedPoint.y > bounds.m_upperBound.y)
				bounds.m_upperBound.y = projectedPoint.y;
			if(projectedPoint.z > bounds.m_upperBound.z)
				bounds.m_upperBound.z = projectedPoint.z;
		}

		// Use bounds to find cascade view matrix
		m_cascadeProjections[i] = Matrix4x4f::OrthoMatrix(bounds.m_lowerBound.x, bounds.m_upperBound.x, // Projection
			bounds.m_lowerBound.y, bounds.m_upperBound.y,
			bounds.m_lowerBound.z, bounds.m_upperBound.z);

		m_cascadeZBounds[i] = Vec2f(bounds.m_lowerBound.z, bounds.m_upperBound.z);

		// Pass points so don't have to recalculate in next cascade
		memcpy(previousPoints, points, sizeof(previousPoints));
	}
}

float Light_Directional_Shadowed::GetSplitDistance(unsigned int cascade, float lambda, float zNear, float zFar)
{
	float cascadeRatio = static_cast<float>(cascade) / static_cast<float>(s_numCascades);
	return lambda * zNear * powf(zFar / zNear, cascadeRatio) + (1.0f - lambda) * (zNear + cascadeRatio * (zFar - zNear));
}

void Light_Directional_Shadowed::SetShader(Scene* pScene)
{
	m_pLighting->m_directionalLightShadowedEffectShader.Bind();

	std::stringstream ss;

	for(unsigned int i = 0; i < s_numCascades; i++)
	{
		ss.str("");
		ss << "cascades[" << i << "]";

		m_pLighting->m_directionalLightShadowedEffectShader.SetShaderTexture(ss.str(), m_shadowMaps[i].GetTextureID(), GL_TEXTURE_2D);

		ss.str("");
		ss << "lightBiasViewProjections[" << i << "]";

		m_pLighting->m_directionalLightShadowedEffectShader.SetUniformmat4(ss.str(), (GetBias() * m_cascadeProjections[i] * m_cascadeViews[i]) * pScene->GetInverseViewMatrix());
	}

	m_pLighting->m_directionalLightShadowedEffectShader.BindShaderTextures();

	m_pLighting->m_directionalLightShadowedEffectShader.SetUniform1fv("splitDistances", s_numCascades, m_splitDistances);
	m_pLighting->m_directionalLightShadowedEffectShader.SetUniformv3f("lightColor", m_color);
	m_pLighting->m_directionalLightShadowedEffectShader.SetUniformv3f("lightDirection", pScene->GetNormalMatrix() * m_direction);
	m_pLighting->m_directionalLightShadowedEffectShader.SetUniformf("lightIntensity", m_intensity);
}

Vec3f GetOrientationVector(const Matrix4x4f &viewMatrix)
{
	return -Vec3f(viewMatrix.m_elements[2], viewMatrix.m_elements[6], viewMatrix.m_elements[10]);
}

Vec3f GetUpVector(const Matrix4x4f &viewMatrix)
{
	return Vec3f(viewMatrix.m_elements[1], viewMatrix.m_elements[5], viewMatrix.m_elements[9]);
}

void GetFrustumCornerPoints(Scene* pScene, float zDistance, Vec3f points[4])
{
	float lerpFactor = zDistance / pScene->m_pWin->m_zFar; //zDistance / (pScene->m_pWin->m_zFar - pScene->m_pWin->m_zNear);

	points[0] = Lerp(pScene->GetFrustum().GetCorner(0), pScene->GetFrustum().GetCorner(1), lerpFactor);
	points[1] = Lerp(pScene->GetFrustum().GetCorner(2), pScene->GetFrustum().GetCorner(3), lerpFactor);
	points[2] = Lerp(pScene->GetFrustum().GetCorner(4), pScene->GetFrustum().GetCorner(5), lerpFactor);
	points[3] = Lerp(pScene->GetFrustum().GetCorner(6), pScene->GetFrustum().GetCorner(7), lerpFactor);
}

void Light_Directional_Shadowed::RenderToCascades(Scene* pScene)
{
	pScene->m_enableShaderSwitches = false;

	for(unsigned int c = 0; c < s_numCascades; c++)
	{
		// Bind the depth map FBO
		m_shadowMaps[c].Bind();
		m_shadowMaps[c].SetViewport();

		// ------------------------------ Render Depth Map ------------------------------

		glMatrixMode(GL_PROJECTION);
		m_cascadeProjections[c].GL_Load();
		glMatrixMode(GL_MODELVIEW);

		float rangeOneMore = m_cascadeZBounds[c].y - m_cascadeZBounds[c].x + 1.0f;

		glClearColor(rangeOneMore, rangeOneMore * rangeOneMore, rangeOneMore, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pScene->SetCustomViewMatrix(m_cascadeViews[c]);

		pScene->ExtractFrustum(m_cascadeProjections[c] * m_cascadeViews[c]);

		pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());

		//std::cout << GetBias() * Matrix4x4f::GL_Get_Projection() * m_cascadeViews[c] * Vec3f(16.0f, 16.0f, 16.0f) << std::endl;

		// Bind shadow map rendering shader
		m_pLighting->m_directionalLightStoreMomentsShader.Bind();
		m_pLighting->m_directionalLightStoreMomentsShader.BindShaderTextures();
		m_pLighting->m_directionalLightStoreMomentsShader.SetUniformv2f("zClipPlanes", m_cascadeZBounds[c]);

		// Change cull face to avoid artifacts
		//glCullFace(GL_FRONT);
		glDisable(GL_CULL_FACE);

		// Render scene to the light distance
		//pScene->Render_Distance(m_cascadeZBounds[c].y - m_cascadeZBounds[c].x);

		// Revert to normal cull face
		//glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		Shader::Unbind();

		FBO::Unbind();

		m_shadowMapBlurPingPongs[c].Bind();
		m_shadowMapBlurPingPongs[c].SetViewport();

		// Ortho projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// Doesn't use m_zNear and m_zFar since ortho projections never really need anything adjustable
		glOrtho(0, m_shadowMaps[c].GetWidth(), 0, m_shadowMaps[c].GetHeight(), -1, 1); 
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// ------------------------------ Blur the Shadow Map ------------------------------

		glDisable(GL_DEPTH_TEST);

		m_pLighting->m_VSMblurShader_horizontal.Bind();
		m_pLighting->m_VSMblurShader_horizontal.SetShaderTexture("scene", m_shadowMaps[c].GetTextureID(), GL_TEXTURE_2D);
		m_pLighting->m_VSMblurShader_horizontal.BindShaderTextures();
		m_pLighting->m_VSMblurShader_horizontal.SetUniformf("blurSize", 0.003f); 

		float widthf = static_cast<float>(m_shadowMaps[c].GetWidth());
		float heightf = static_cast<float>(m_shadowMaps[c].GetHeight());
	
		// Fullscreen quad to blur
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
		glTexCoord2i(1, 0); glVertex3f(widthf, 0.0f, 0.0f);
		glTexCoord2i(1, 1); glVertex3f(widthf, heightf, 0.0f);
		glTexCoord2i(0, 1); glVertex3f(0.0f, heightf, 0.0f);
		glEnd();

		FBO::Unbind();
		m_shadowMaps[c].Bind();

		m_pLighting->m_VSMblurShader_vertical.Bind();
		m_pLighting->m_VSMblurShader_vertical.SetShaderTexture("scene", m_shadowMapBlurPingPongs[c].GetTextureID(), GL_TEXTURE_2D);
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
	}
	
	// ------------------------------ Reset ------------------------------

	Shader::Unbind();

	pScene->m_pWin->SetViewport();
	pScene->m_pWin->SetProjection();

	// Unbind depth map FBO
	FBO::Unbind();

	pScene->m_enableShaderSwitches = true;

	GL_ERROR_CHECK();
}

void Light_Directional_Shadowed::DebugDrawCascade(Scene* pScene, unsigned int index)
{
	assert(index < s_numCascades);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix4x4f::OrthoMatrix(0.0f, pScene->m_pWin->m_projected_width, 0.0f, pScene->m_pWin->m_projected_height, -1.0f, 1.0f).GL_Mult();
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_shadowMapBlurPingPongs[index].GetTextureID());
	DrawQuadOriginBottomLeft(256.0f, 256.0f);
	glEnable(GL_DEPTH_TEST);

	pScene->m_pWin->SetProjection();
}