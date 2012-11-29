#pragma once

#include <AssetManager/Asset.h>

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/BufferObjects/VBO.h>
#include <AssetManager/Asset_Texture.h>

#include <Constructs/AABB.h>

class Model_OBJ_VertexOnly :
	public Asset
{
private:
	VBO m_vertices;
	VBO m_indices;

	unsigned short m_numVertices;

	bool m_loaded;

	AABB m_aabb;
	Vec3f m_aabbOffsetFromModel;

public:
	Model_OBJ_VertexOnly();
	virtual ~Model_OBJ_VertexOnly() {}

	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	void Render();

	bool Loaded();

	AABB GetAABB();

	const Vec3f &GetAABBOffsetFromModel();

	// Asset factory
	static Asset* Asset_Factory();
};

