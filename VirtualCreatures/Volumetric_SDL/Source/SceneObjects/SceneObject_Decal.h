#pragma once

#include <AssetManager/Asset_Texture.h>

#include <Constructs/Vec3f.h>

#include <Scene/SceneObject.h>

#include <SceneObjects/Physics/SceneObject_Prop_Physics_Dynamic.h>

class SceneObject_Decal :
	public SceneObject
{
private:
	Asset_Texture* m_pDecalTexture_diffuse;
	Asset_Texture* m_pDecalTexture_specular;
	Asset_Texture* m_pDecalTexture_normal;

	Matrix4x4f m_transform;

	float m_age;
	float m_despawnTime;
	
	float m_halfWidth, m_halfHeight;

	SceneObjectReferenceTracker m_propTracker;
	SceneObject_Prop_Physics_Dynamic* m_pProp;

	class SceneObject_Decal_BatchRenderer* m_pDecalRenderer;

	void Render_Batch_NoTexture();
	void Render_Batch_Textured();

public:
	float m_specularColor;

	SceneObject_Decal();

	bool Create(const std::string &fileName, const Vec3f &position, const Vec3f &direction, float despawnTime, float spriteWidth);
	bool Create(const std::string &fileName, SceneObject_Prop_Physics_Dynamic* pProp, const Vec3f &position, const Vec3f &direction, float despawnTime, float spriteWidth);

	bool AddSpecularMap(const std::string &fileName);
	bool AddNormalMap(const std::string &fileName);

	// Inherited from SceneObject
	void Logic();
	void Render();

	friend class SceneObject_Decal_BatchRenderer;
};

class SceneObject_Decal_BatchRenderer :
	public BatchRenderer
{
private:
	// All decals
	std::vector<SceneObject_Decal*> m_pDecals;

	unsigned char m_decalIndices[8];

public:
	SceneObject_Decal_BatchRenderer();

	void Add(SceneObject_Decal* pDecal);

	// Inherited from BatchRenderer
	void Execute();
	void Clear();

	static BatchRenderer* SceneObject_Decal_BatchRendererFactory();
};