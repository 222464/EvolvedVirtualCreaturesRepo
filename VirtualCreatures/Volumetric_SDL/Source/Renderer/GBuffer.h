#pragma once

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Window.h>
#include <Renderer/Shader/Shader.h>

class GBuffer :
	public Uncopyable
{
private:
	unsigned int m_width, m_height;

	// Array contains textures for the different buffer types
	static const unsigned int m_numBufferTextures = 3;
	static const unsigned int m_numBuffersAndEffect = m_numBufferTextures + 1;
	static const unsigned int m_effectTextureAttachment = m_numBufferTextures;

	unsigned int m_drawBuffers[m_numBuffersAndEffect];
	
	unsigned int m_gTextureIDs[m_numBufferTextures];
	unsigned int m_fboID;
	unsigned int m_depthTextureID;
	unsigned int m_effectTextureID;

	bool m_created;

public:
	enum BufferType
	{
		e_position = 0, e_normal, e_color
	};

	GBuffer();
	~GBuffer();

	void Create(unsigned int width, unsigned int height);
	void Create(const Window &window);
	
	// Rendering to GBuffer
	void SetViewport();

	void Bind();
	void Bind_Draw();
	static void Unbind_Draw();

	void Set_DrawGeom();
	void Finish_Draw();

	// Reading from GBuffer
	static void Unbind_Read();
	void Bind_Read();

	static void Unbind();

	void Set_DrawNone();

	void SetReadBuffer(BufferType type);
	void SetDraw_Effect();
	void SetRead_Effect();

	void CopyEffectToMainFrameBuffer();

	unsigned int GetTextureID(BufferType type);

	bool Created();

	unsigned int GetWidth();
	unsigned int GetHeight();

	void DrawBufferSizedQuad();

	void DrawAsQuad(BufferType type, class Scene* pScene);
	void DrawEffectAsQuad(class Scene* pScene);

	unsigned int GetEffectTextureID();
	unsigned int GetDepthTextureID();
};

