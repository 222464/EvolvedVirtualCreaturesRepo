#pragma once

#include <Renderer/SDL_OpenGL.h>

#include <Renderer/Shader/Shader.h>

#include <Renderer/BufferObjects/FBO.h>

#include <Scene/Scene.h>

class ShadowCubeMap
{
private:
	Scene* m_pScene;

	Shader* m_pMomentStoreShader;
	Shader* m_pHorizontalVSMBlur;
	Shader* m_pVerticalVSMBlur;

	unsigned int m_fboID;

	unsigned int m_cubeMapID;

	unsigned int m_resolution;

	FBO m_blurTempFBO;

	void SetPerspective();
	void SetOrtho();

	void DrawFace(unsigned int face, const Vec3f &position, const Vec3f &dir, float distance);

	bool m_created;

public:
	ShadowCubeMap();
	~ShadowCubeMap();

	void Create(unsigned int resolution, Scene* pScene, Shader* pMomentStoreShader, Shader* pHorizontalVSMBlur, Shader* pVerticalVSMBlur);
	void Destroy();

	void Render(const Vec3f &position, Scene* pScene, Shader* pMomentStoreShader, Shader* pHorizontalVSMBlur, Shader* pVerticalVSMBlur, float distance);

	unsigned int GetTextureID();
};

