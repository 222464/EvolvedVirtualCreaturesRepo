#pragma once

#include <Scene/SceneObject.h>

#include <World/World.h>

class SceneObject_Water :
	public SceneObject
{
private:
	World* m_pWorld;

	Shader* m_pDeferredWaterShader;

	Asset_Texture* m_pBumpMap0, * m_pBumpMap1;

	unsigned int m_numVertices;

	VBO m_vertexBuffer;
	VBO m_texCoordBuffer;
	VBO m_normalBuffer;

	void ExpandInWorld(const Point3i worldStartVoxel);
	void AddGeometry(std::vector<Vec3f> &positions, std::vector<Vec3f> &normals, std::vector<Vec2f> &texCoords, const Vec3f &position, const Vec2f &changeInPosition);

public:
	static bool s_waterShaderSetup;

	static const int s_numWaterBlocksLimit = 2000;
	static const int s_waterBlockID = 128;
	static const float s_waterSurfaceOffset;
	static const float s_texScalar;

	float m_bumpMapDriftSpeed;
	float m_bumpMapOffset;

	SceneObject_Water();
	~SceneObject_Water();

	bool Create(const std::string &bumpMap0Name, const std::string &bumpMap1Name, const Point3i worldStartVoxel);

	// Inherited from SceneObject
	void Logic();
	void Render();
};

