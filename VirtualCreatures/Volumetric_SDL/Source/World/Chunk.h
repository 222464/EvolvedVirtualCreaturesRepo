#pragma once

#include <Constructs/Point3i.h>
#include <Constructs/Vec3f.h>

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/BufferObjects/VBO.h>
#include <Renderer/Octree/OctreeOccupant.h>
#include <Renderer/BufferObjects/FBO.h>
#include <Renderer/Shader/Shader.h>

#include <Scene/SceneObject.h>
#include <Scene/BatchRenderer.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <string>

class World;

class Chunk
{
private:
	AABB m_aabb;

	VBO m_vertexBuffer;
	VBO m_texCoordBuffer;
	VBO m_colorBuffer;
	VBO m_normalBuffer;

	unsigned int m_numVertices;
	
	World* m_pWorld;

	void AddGeometry(const Point3i &lower, int side, std::vector<Vec3f> &vertexArray, char voxelType);
	inline void AddTexCoord(std::vector<Vec2f> &texCoordArray, unsigned char id, int side);
	inline void AddColor(Color3f occlusionFactor[4], int side, std::vector<Color3f> &colorArray);
	inline void AddNormals(std::vector<Vec3f> &normalArray, int side);

	unsigned short PositionToIndex(const Point3i &position);

	void GetSemiSphereOcclusion(const Vec3f &blockPos, int side, Color3f data[4]);

	bool m_empty;

	bool m_created;

	Point3i m_matrixPos;
	Vec3f m_worldPos;

	// Physics
	btTriangleMesh* m_pTriangleMesh;
	btBvhTriangleMeshShape* m_pMeshShape;
	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

	class Chunk_Renderer* m_pChunkRenderer;

	bool m_physicsCreated;

public:
	static const int s_chunkSizeX = 16;
	static const int s_chunkSizeY = 16;
	static const int s_chunkSizeZ = 16;

	static const int s_onePastChunkSizeX = s_chunkSizeX + 1;
	static const int s_onePastChunkSizeY = s_chunkSizeY + 1;
	static const int s_onePastChunkSizeZ = s_chunkSizeZ + 1;

	static const int s_numVertices = s_onePastChunkSizeX * s_onePastChunkSizeY * s_onePastChunkSizeZ;

	static unsigned char s_numTileTexturesInX;
	static unsigned char s_numTileTexturesInY;
	static unsigned char s_numTileTextures;

	static float s_friction;

	unsigned char m_voxelMatrix[s_chunkSizeX][s_chunkSizeY][s_chunkSizeZ];

	Chunk();
	~Chunk();

	void Create(World* pWorld, const Point3i &matrixPos);

	void GenerateRenderAndPhysicsData();
	void Draw();

	float GetRayCollisionDist(const Vec3f &start, const Vec3f &dir, float maxDist); // Returns -1.0f of the ray didn't hit anything

	bool IsEmpty();

	static int GetSideFromDir(const Point3i &dir);

	friend World;
	friend class Chunk_Renderer;
};

class Chunk_Renderer :
	public SceneObject
{
public:
	Chunk* m_pChunk;

	Chunk_Renderer(Chunk* pChunk, const AABB &aabb);

	// Inherited from SceneObject
	void Render();
};

class Chunk_BatchRenderer :
	public BatchRenderer
{
private:
	std::list<Chunk_Renderer*> m_chunks;
public:
	World* m_pWorld;

	Chunk_BatchRenderer();

	void Add(Chunk_Renderer* pChunkRenderer);

	// Inherited from BatchRenderer
	void Execute();
	void Clear();

	static BatchRenderer* Chunk_BatchRendererFactory();
};