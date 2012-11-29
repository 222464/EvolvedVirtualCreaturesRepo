#pragma once

#include <Scene/Scene.h>
#include <World/Chunk.h>
#include <Renderer/Octree/StaticOctree.h>
#include <Renderer/Shader/Shader.h>
#include <Renderer/Window.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <AssetManager/Asset_Texture.h>

#include <System/CSDLThreadPool.h>

#include <Constructs/Point3i.h>

#include <vector>

class PathFindingTask :
	public Task
{
private:
	World* m_pWorld;

public:
	std::vector<Point3i> m_path;

	Point3i m_start, m_end;

	unsigned int m_maxSteps;

	PathFindingTask()
		: m_maxSteps(512)
	{
	}

	// Inherited from Task
	void Run();

	friend class World;
};

// Uncomment this to make the chunks solid on the outside (does not generate geometry at end of map)
#define OUTSIDE_CHUNKS_SOLID

struct SideTexCoords
{
	float m_texCoord_x_l, m_texCoord_x_h, m_texCoord_y_l, m_texCoord_y_h;
};

class VoxelFaceDesc
{
private:
	int m_top, m_bottom, m_xzSides;

	// Array holding a texture coordinates for all sides
	SideTexCoords m_sideTexCoords[6];

public:
	VoxelFaceDesc();
	VoxelFaceDesc(int top, int bottom, int xzSides);

	void Create(int top, int bottom, int xzSides);

	// Assigns texture coordinates for given top, bottom, and xzSides
	void AddCoords(int side, std::vector<Vec2f> &texCoords);
};

class World :
	public SceneObject
{
private:
	std::vector<std::vector<std::vector<Chunk>>> m_chunkMatrix;

	int m_chunksInX, m_chunksInY, m_chunksInZ;

	bool m_created;

	// Normals for faces - generated in ctor
	Vec3f m_normals[6];

	// Chunk filled test direction offsets - generated in ctor
	Point3i m_positionOffsets[6];

	// Pre-calculated corners
	Vec3f m_corners[6][4];

	static const unsigned int s_numAOSamples = 32;
	float m_AOSampleDistance;
	float m_AOSampleAngle;

	Vec3f m_sphereDistribution[s_numAOSamples];

	void GetSphereDistribution(Vec3f points[s_numAOSamples]);

	unsigned int m_chunkOcclusionQueryID;

	// Texture for all voxels
	Asset_Texture m_voxelTex_diffuse;
	Asset_Texture m_voxelTex_specular;
	Asset_Texture m_voxelTex_bump;

	// Texture file description - relates each index to 6 "real" indices, 1 for each face
	std::vector<VoxelFaceDesc> m_voxelTexturePosDescs;

	Point3i GetRayCollisionDist(const Vec3f &start, const Vec3f &dir, float castDist); // Returns a point (-1, -1, -1) of the ray didn't hit anything

	// Returns true if loaded properly, else false
	bool LoadVoxelFaceTextureDesc(const std::string &fileName);
	
	int GetRealTextureIndex(int id, int side);

	Chunk_BatchRenderer* m_pBatchRenderer;

	// Physics
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	// Path finding
	CSDLThreadPool m_pathFindingThreadPool;

	void DestroyChunks();

public:
	Vec3f m_lightVec;
	float m_ambient;

	World();
	~World();

	bool Create(int chunksInX, int chunksInY, int chunksInZ, const std::string &diffuseTextureFilePath, const std::string &specularTextureFilePath, const std::string &bumpTextureFilePath, const std::string &textureDescFilePath);

	int GetChunksInX() const;
	int GetChunksInY() const;
	int GetChunksInZ() const;

	unsigned char GetVoxel(int x, int y, int z);
	void SetVoxel(int x, int y, int z, unsigned char type);

	// For use with pathfinding only - doesn't update empty chunks!
	void SetVoxel_NoCheck(int x, int y, int z, unsigned char type);
	void IncVoxel(int x, int y, int z, char increment);

	void SetVoxelAndUpdate(const Vec3f &viewerDir, int x, int y, int z, unsigned char type);

	// Returns voxel pointed at
	Point3i HighlightVoxel(const Vec3f &viewerPos, const Vec3f &viewerDir);

	void GenerateAllChunks();

	void FillBox(const Point3i &lowerBound, const Point3i &upperBound, unsigned char id);

	// Inherited from SceneObject
	void OnAdd();

	void SetMaterial();
	void ResetMaterial();

	// Used to render mini map
	void RenderAllChunks();

	// dir_xz is cardinal direction from 0 to 3, dir_y is cardinal direction from -1 to 1
	Point3i TransformPoint3i(const Point3i &point, int dir_xz);

	void StartPathFindingTask(PathFindingTask* pTask);

	friend class Chunk;
	friend class Chunk_Renderer;
};

bool DistCompare(OctreeOccupant* first, OctreeOccupant* second);