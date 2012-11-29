#include <Renderer/ShadowMap/ShadowCubeMap.h>

#include <Utilities/UtilFuncs.h>

#include <assert.h>

#include <Renderer/RenderUtils.h>

ShadowCubeMap::ShadowCubeMap()
	: m_created(false)
{
}

ShadowCubeMap::~ShadowCubeMap()
{
	Destroy();
}

void ShadowCubeMap::Create(unsigned int resolution, Scene* pScene, Shader* pMomentStoreShader, Shader* pHorizontalVSMBlur, Shader* pVerticalVSMBlur)
{
	assert(!m_created);

#ifdef DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	unsigned int uResult = static_cast<unsigned>(result);
	assert(resolution > 0 && resolution <= uResult);
#endif

	m_resolution = resolution;

	m_pScene = pScene;
	m_pMomentStoreShader = pMomentStoreShader;
	m_pHorizontalVSMBlur = pHorizontalVSMBlur;
	m_pVerticalVSMBlur = pVerticalVSMBlur;

	// ------------------------- Depth Texture ---------------------------

	glGenTextures(1, &m_cubeMapID);

	// Bind as cube map and set default settings
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
	// Generate textures for faces
	for(int f = 0; f < 6; f++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGB16F, m_resolution, m_resolution, 0, GL_RGB, GL_FLOAT, NULL);

	// Generate mipmaps for the color texture
	//glGenerateMipmap(GL_TEXTURE_2D);

	GL_ERROR_CHECK();

	// ------------------------------ FBO ------------------------------

	glGenFramebuffers(1, &m_fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

	// Not using color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// Attach at least on part of cube map so completeness check succeeds. Attach and detach sides later when rendering
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubeMapID, 0);
	
	GL_ERROR_CHECK();

#ifdef DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not created FBO!" << std::endl;
#endif

	// Unbind FBO, revert to main framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Unbind texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	m_blurTempFBO.Create(resolution, resolution, false, GL_RGB16F, GL_RGB, GL_FLOAT);

	GL_ERROR_CHECK();

	m_created = true;
}

void ShadowCubeMap::Destroy()
{
	assert(m_created);

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_cubeMapID);

	m_created = false;
}

void ShadowCubeMap::Render(const Vec3f &position, Scene* pScene, Shader* pMomentStoreShader, Shader* pHorizontalVSMBlur, Shader* pVerticalVSMBlur, float distance)
{
	// Unbind any existing textures
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// For each face, render the scene
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

	// Set up viewport for the map side
	glViewport(0, 0, m_resolution, m_resolution);

	SetPerspective();

	//glColorMask(false, false, false, false);

	glClearDepth(1.0f);

	// Side +X
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubeMapID, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x + 1.0f, position.y, position.z, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();

	pMomentStoreShader->Bind();

	pScene->Render_Distance(distance);

	pMomentStoreShader->Unbind();

	m_blurTempFBO.Bind();

	DrawNormalizedQuad();

	// Side -X
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, m_cubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x - 1.0f, position.y, position.z, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();
	pScene->Render_Distance(distance);

	// Side +Y
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, m_cubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x, position.y + 1.0f, position.z, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();
	pScene->Render_Distance(distance);

	// Side -Y
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, m_cubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x, position.y - 1.0f, position.z, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();
	pScene->Render_Distance(distance);

	// Side +Z
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_cubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x, position.y, position.z + 1.0f, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();
	pScene->Render_Distance(distance);

	// Side -Z
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, m_cubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z, position.x, position.y, position.z - 1.0f, 0.0f, 1.0f, 0.0f);
	pScene->ExtractFrustum();
	pScene->Render_Distance(distance);

	//glColorMask(true, true, true, true);

	glClearDepth(1.0f);

	

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapID);

	float buf;

	glReadPixels( 512, 512, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buf);

	std::cout << buf << std::endl;
	// Unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERROR_CHECK();
}

unsigned int ShadowCubeMap::GetTextureID()
{
	return m_cubeMapID;
}

void ShadowCubeMap::SetPerspective()
{
	// Set projection to 90 degrees
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0f, 1.0f, 0.01f, 2000.0f);
	glMatrixMode(GL_MODELVIEW);
}

void ShadowCubeMap::SetOrtho()
{
	// Set projection to 90 degrees
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

void ShadowCubeMap::DrawFace(unsigned int face, const Vec3f &position, const Vec3f &dir, float distance)
{
	// Side +X
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, m_cubeMapID, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	Matrix4x4f::TranslateMatrix(-position).GL_Mult();
	Matrix4x4f::CameraDirectionMatrix(dir, Vec3f(0.0f, 1.0f, 0.0f)).GL_Mult();
	m_pScene->ExtractFrustum();

	m_pMomentStoreShader->Bind();

	m_pScene->Render_Distance(distance);

	m_pMomentStoreShader->Unbind();

	m_blurTempFBO.Bind();

	DrawNormalizedQuad();
}