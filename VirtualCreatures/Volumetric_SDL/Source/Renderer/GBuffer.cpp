#include <Renderer/GBuffer.h>

#include <iostream>

#include <Scene/Scene.h>

#include <assert.h>

GBuffer::GBuffer()
	: m_created(false)
{
}

GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(m_numBufferTextures, m_gTextureIDs);
	glDeleteTextures(1, &m_depthTextureID);
	glDeleteTextures(1, &m_effectTextureID);
}

void GBuffer::Create(unsigned int width, unsigned int height)
{
	m_width = width;
	m_height = height;

	// Create the FBO
	glGenFramebuffers(1, &m_fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

	// Create all GBuffer textures
	glGenTextures(m_numBufferTextures, m_gTextureIDs);

	// Position
	glBindTexture(GL_TEXTURE_2D, m_gTextureIDs[e_position]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Normal
	glBindTexture(GL_TEXTURE_2D, m_gTextureIDs[e_normal]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Color
	glBindTexture(GL_TEXTURE_2D, m_gTextureIDs[e_color]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Attach the textures to the FBO
	for(unsigned int i = 0; i < m_numBufferTextures; i++)
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_gTextureIDs[i], 0);

	// Create depth texture with stencil
	glGenTextures(1, &m_depthTextureID);
	glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_width, m_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	// Make it readable in a shader
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);

	// Create the effect buffer
	glGenTextures(1, &m_effectTextureID);
	glBindTexture(GL_TEXTURE_2D, m_effectTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_effectTextureAttachment, GL_TEXTURE_2D, m_effectTextureID, 0);

	// Set which buffers to draw to
	for(unsigned int i = 0; i < m_numBuffersAndEffect; i++)
		m_drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

    glDrawBuffers(m_numBuffersAndEffect, m_drawBuffers);

	// Check that the buffer was properly created
#ifdef DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not created FBO!" << std::endl;
#endif

	// Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERROR_CHECK();

	CheckForGLError();

	m_created = true;
}

void GBuffer::Create(const Window &window)
{
	Create(window.GetPixelWidth(), window.GetPixelHeight());
}

bool GBuffer::Created()
{
	return m_created;
}

void GBuffer::SetViewport()
{
	glViewport(0, 0, m_width, m_height);
}

void GBuffer::Bind()
{
	assert(m_created);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
}

void GBuffer::Bind_Draw()
{
	assert(m_created);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void GBuffer::Bind_Read()
{
	assert(m_created);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
}

void GBuffer::Unbind_Draw()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GBuffer::Unbind_Read()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Set_DrawGeom()
{
	// Set which buffers to draw to
    glDrawBuffers(m_numBuffersAndEffect, m_drawBuffers);
}

void GBuffer::Finish_Draw()
{
	
}

void GBuffer::Set_DrawNone()
{
	glDrawBuffer(GL_NONE);
}

void GBuffer::SetReadBuffer(BufferType type)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + type);
}

void GBuffer::SetDraw_Effect()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + m_effectTextureAttachment);
}

void GBuffer::SetRead_Effect()
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + m_effectTextureAttachment);
}

void GBuffer::CopyEffectToMainFrameBuffer()
{
	Unbind_Draw();

	SetRead_Effect();

	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	Bind_Draw();
}

unsigned int GBuffer::GetTextureID(BufferType type)
{
	return m_gTextureIDs[type];
}

unsigned int GBuffer::GetWidth()
{
	return m_width;
}

unsigned int GBuffer::GetHeight()
{
	return m_height;
}

void GBuffer::DrawBufferSizedQuad()
{
	float widthf = static_cast<float>(m_width);
	float heightf = static_cast<float>(m_height);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex3f(widthf, 0.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex3f(widthf, heightf, 0.0f);
	glTexCoord2i(0, 1); glVertex3f(0.0f, heightf, 0.0f);
	glEnd();
}

void GBuffer::DrawAsQuad(BufferType type, Scene* pScene)
{
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_gTextureIDs[type]);

	pScene->m_pWin->SetOrtho();
	glLoadIdentity();
	DrawBufferSizedQuad();
	pScene->m_pWin->SetProjection();

	// Matrix reset
	pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
}

void GBuffer::DrawEffectAsQuad(Scene* pScene)
{
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_effectTextureID);

	pScene->m_pWin->SetOrtho();
	glLoadIdentity();
	DrawBufferSizedQuad();
	pScene->m_pWin->SetProjection();

	// Matrix reset
	pScene->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
}

unsigned int GBuffer::GetEffectTextureID()
{
	return m_effectTextureID;
}

unsigned int GBuffer::GetDepthTextureID()
{
	return m_depthTextureID;
}