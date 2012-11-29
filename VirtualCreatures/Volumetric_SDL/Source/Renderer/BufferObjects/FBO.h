#pragma once

#include <Renderer/SDL_OpenGL.h>

class FBO
{
private:
	unsigned int m_width, m_height;

	unsigned int m_fboID, m_texID, m_depthID; // m_depthID is set to -1 to indicate that it is not used

	bool m_created;

	void Create_FromSetParams(bool depth, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType);

public:
	FBO();
	FBO(unsigned int width, unsigned int height, bool depthAttachment, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType);
	~FBO();

	void Create(unsigned int width, unsigned int height, bool depthAttachment, unsigned int internalFormat, unsigned int textureFormat, unsigned int dataType);
	void Destroy();

	unsigned int GetWidth();
	unsigned int GetHeight();

	void SetViewport();
	void Bind();
	void Bind_Draw();
	void Bind_Read();
	static void Unbind();
	static void Unbind_Draw();
	static void Unbind_Read();
	unsigned int GetTextureID();
	unsigned int GetFBOID();

	void DrawBufferSizedQuad();

	bool Created();
};

