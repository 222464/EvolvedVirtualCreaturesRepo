#include <Renderer/BufferObjects/FBO.h>

#include <assert.h>

#include <iostream>

FBO::FBO()
	: m_created(false)
{
}

FBO::FBO(unsigned int width, unsigned int height, bool depthAttachment, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType)
	: m_created(false),
	m_width(width), m_height(height)
{
	Create_FromSetParams(depthAttachment, internalFormat, textureFormat, dataType);
}

FBO::~FBO()
{
	if(m_created)
		Destroy();
}

void FBO::Create_FromSetParams(bool depth, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType)
{
#ifdef DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	unsigned int uResult = static_cast<unsigned>(result);
	assert(m_width > 0 && m_height > 0 && m_width <= uResult && m_height <= uResult);
#endif

	// Generate the FBO
	glGenFramebuffers(1, &m_fboID);

	// Create a texture to attach to it
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	
	// Default settings for the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create empty texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, textureFormat, dataType, NULL);

	GL_ERROR_CHECK();

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
		
	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, m_texID, 0); // The 0 is for mip map levels, we aren't using any

	GL_ERROR_CHECK();

	// If depth buffer attachment was desired
	if(depth)
	{
		glGenRenderbuffers(1, &m_depthID);
			
		// Bind it so we can set it up
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthID);
			
		// Set up the depth buffer
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		
		// Attach the dpeth buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthID);

		GL_ERROR_CHECK();
	}
	else
		m_depthID = 0; // Unused

	// Check that the buffer was properly created
#ifdef DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create FBO!" << std::endl;
#endif
	
	// Unbind
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_created = true;
}

void FBO::Create(unsigned int width, unsigned int height, bool depthAttachment, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType)
{
	m_width = width;
	m_height = height;

	Create_FromSetParams(depthAttachment, internalFormat, textureFormat, dataType);
}

void FBO::Destroy()
{
	assert(m_created);

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_texID);

	// If depth attachment was used
	if(m_depthID != -1)
		glDeleteRenderbuffers(1, &m_depthID);

	m_created = false;
}

void FBO::SetViewport()
{
	glViewport(0, 0, m_width, m_height);
}

void FBO::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
}

void FBO::Bind_Draw()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void FBO::Bind_Read()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
}

void FBO::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::Unbind_Draw()
{	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FBO::Unbind_Read()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

unsigned int FBO::GetTextureID()
{
	return m_texID;
}

unsigned int FBO::GetFBOID()
{
	return m_fboID;
}

unsigned int FBO::GetWidth()
{
	return m_width;
}

unsigned int FBO::GetHeight()
{
	return m_height;
}

bool FBO::Created()
{
	return m_created;
}

void FBO::DrawBufferSizedQuad()
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