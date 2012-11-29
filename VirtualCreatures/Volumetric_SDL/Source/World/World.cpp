#include <World/World.h>

#include <Utilities/UtilFuncs.h>

#include <Perlin/NoiseGenerator.h>

#include <PathFinding/AStar.h>

#include <assert.h>

#include <sstream>

#include <fstream>

#include <algorithm>

Vec3f viewerPosition;

VoxelFaceDesc::VoxelFaceDesc()
{

}

VoxelFaceDesc::VoxelFaceDesc(int top, int bottom, int xzSides)
	: m_top(top), m_bottom(bottom), m_xzSides(xzSides)
{
	float width = 1.0f / static_cast<float>(Chunk::s_numTileTexturesInX);
	float height = 1.0f / static_cast<float>(Chunk::s_numTileTexturesInY);

	for(int i = 0; i < 6; i++)
	{
		int texID;

		switch(i)
		{	
		case 3: // Bottom
			texID = m_bottom;
			break;
		case 2: // Top
			texID = m_top;
			break;
		default: // xz faces
			texID = m_xzSides;
			break;
		}
	
		int xiFromID = texID % Chunk::s_numTileTexturesInX;
		int yiFromID = texID / Chunk::s_numTileTexturesInX;

		assert(yiFromID < Chunk::s_numTileTexturesInY);

		m_sideTexCoords[i].m_texCoord_x_l = xiFromID * width;
		m_sideTexCoords[i].m_texCoord_y_l = yiFromID * height;
		m_sideTexCoords[i].m_texCoord_x_h = m_sideTexCoords[i].m_texCoord_x_l + width;
		m_sideTexCoords[i].m_texCoord_y_h = m_sideTexCoords[i].m_texCoord_y_l + height;
	}
}

void VoxelFaceDesc::Create(int top, int bottom, int xzSides)
{
	m_top = top;
	m_bottom = bottom;
	m_xzSides = xzSides;

	float width = 1.0f / static_cast<float>(Chunk::s_numTileTexturesInX);
	float height = 1.0f / static_cast<float>(Chunk::s_numTileTexturesInY);

	for(int i = 0; i < 6; i++)
	{
		int texID;

		switch(i)
		{	
		case 3: // Bottom
			texID = m_bottom;
			break;
		case 2: // Top
			texID = m_top;
			break;
		default: // xz faces
			texID = m_xzSides;
			break;
		}
	
		int xiFromID = texID % Chunk::s_numTileTexturesInX;
		int yiFromID = texID / Chunk::s_numTileTexturesInX;

		assert(yiFromID < Chunk::s_numTileTexturesInY);

		m_sideTexCoords[i].m_texCoord_x_l = xiFromID * width;
		m_sideTexCoords[i].m_texCoord_y_l = yiFromID * height;
		m_sideTexCoords[i].m_texCoord_x_h = m_sideTexCoords[i].m_texCoord_x_l + width;
		m_sideTexCoords[i].m_texCoord_y_h = m_sideTexCoords[i].m_texCoord_y_l + height;
	}
}

// Assigns texture coordinates for given top, bottom, and xzSides
void VoxelFaceDesc::AddCoords(int side, std::vector<Vec2f> &texCoords)
{
	texCoords.push_back(Vec2f(m_sideTexCoords[side].m_texCoord_x_l, m_sideTexCoords[side].m_texCoord_y_h));
	texCoords.push_back(Vec2f(m_sideTexCoords[side].m_texCoord_x_h, m_sideTexCoords[side].m_texCoord_y_h));
	texCoords.push_back(Vec2f(m_sideTexCoords[side].m_texCoord_x_h, m_sideTexCoords[side].m_texCoord_y_l));
	texCoords.push_back(Vec2f(m_sideTexCoords[side].m_texCoord_x_l, m_sideTexCoords[side].m_texCoord_y_l));
}

void PathFindingTask::Run()
{
	GetPath_LowMemory(m_path, m_pWorld, m_start, m_end, AddSuccessorFunc_Walking, HeuristicFunc_ManhattanDistance, m_maxSteps);
}

World::World()
	: m_created(false),
	m_lightVec(0.4f, -0.5f, -0.7f),
	m_ambient(0.1f),
	m_pPhysicsWorld(NULL), m_pathFindingThreadPool(8), m_AOSampleDistance(6.0f),
	m_AOSampleAngle(pif / s_numAOSamples)
{
	m_lightVec = m_lightVec.Normalize();

	// Generate normals
	m_normals[0] = Vec3f(1.0f, 0.0f, 0.0f);
	m_normals[1] = Vec3f(-1.0f, 0.0f, 0.0f);
	m_normals[2] = Vec3f(0.0f, 1.0f, 0.0f);
	m_normals[3] = Vec3f(0.0f, -1.0f, 0.0f);
	m_normals[4] = Vec3f(0.0f, 0.0f, 1.0f);
	m_normals[5] = Vec3f(0.0f, 0.0f, -1.0f);

	// Generate test offsets
	m_positionOffsets[0] = Point3i(1, 0, 0);
	m_positionOffsets[1] = Point3i(-1, 0, 0);
	m_positionOffsets[2] = Point3i(0, 1, 0);
	m_positionOffsets[3] = Point3i(0, -1, 0);
	m_positionOffsets[4] = Point3i(0, 0, 1);
	m_positionOffsets[5] = Point3i(0, 0, -1);

	// Generate corners
	float cornerDist = 0.5f;

	m_corners[0][0] = Vec3f(cornerDist, -cornerDist, cornerDist);
	m_corners[0][1] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	m_corners[0][2] = Vec3f(cornerDist, cornerDist, -cornerDist);
	m_corners[0][3] = Vec3f(cornerDist, cornerDist, cornerDist);

	m_corners[1][0] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	m_corners[1][1] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	m_corners[1][2] = Vec3f(-cornerDist, cornerDist, cornerDist);
	m_corners[1][3] = Vec3f(-cornerDist, cornerDist, -cornerDist);

	m_corners[2][0] = Vec3f(-cornerDist, cornerDist, -cornerDist);
	m_corners[2][1] = Vec3f(-cornerDist, cornerDist, cornerDist);
	m_corners[2][2] = Vec3f(cornerDist, cornerDist, cornerDist);
	m_corners[2][3] = Vec3f(cornerDist, cornerDist, -cornerDist);

	m_corners[3][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	m_corners[3][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	m_corners[3][2] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	m_corners[3][3] = Vec3f(cornerDist, -cornerDist, cornerDist);

	m_corners[4][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	m_corners[4][1] = Vec3f(cornerDist, -cornerDist, cornerDist);
	m_corners[4][2] = Vec3f(cornerDist, cornerDist, cornerDist);
	m_corners[4][3] = Vec3f(-cornerDist, cornerDist, cornerDist);

	m_corners[5][0] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	m_corners[5][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	m_corners[5][2] = Vec3f(-cornerDist, cornerDist, -cornerDist);
	m_corners[5][3] = Vec3f(cornerDist, cornerDist, -cornerDist);

	// Generate chunk occlusion query
	glGenQueriesARB(1, &m_chunkOcclusionQueryID);

	GL_ERROR_CHECK();

	m_unmanagedName = "world";

	GetSphereDistribution(m_sphereDistribution);
}

World::~World()
{
	glDeleteQueriesARB(1, &m_chunkOcclusionQueryID);
}

// Spherical distribution algorithm from http://www.softimageblog.com/archives/115
void World::GetSphereDistribution(Vec3f points[s_numAOSamples])
{
	float nf = static_cast<float>(s_numAOSamples);

	float increment = pif * (3.0f - sqrtf(5));

	float off = 2.0f / nf;

	for(int k = 0; k < s_numAOSamples; k++)
	{
		float y = k * off - 1.0f + off / 2.0f;
		float r = sqrtf(1.0f - y * y);
		float phi = k * increment; // ) / 2.0f; // divide by 2 to get semi-sphere

		points[k] = Vec3f(cosf(phi) * r, y, sinf(phi) * r);
	}
}

bool World::Create(int chunksInX, int chunksInY, int chunksInZ, const std::string &diffuseTextureFilePath, const std::string &specularTextureFilePath, const std::string &bumpTextureFilePath, const std::string &textureDescFilePath)
{
	assert(!m_created);

	// Load the voxel textures
	if(!m_voxelTex_diffuse.LoadAsset(diffuseTextureFilePath))
	{
		std::cerr << "Failed to create world: Could not load " << diffuseTextureFilePath << "!" << std::endl;
		return false;
	}

	if(!m_voxelTex_specular.LoadAsset(specularTextureFilePath))
	{
		std::cerr << "Failed to create world: Could not load " << specularTextureFilePath << "!" << std::endl;
		return false;
	}
	
	if(!m_voxelTex_bump.LoadAsset(bumpTextureFilePath))
	{
		std::cerr << "Failed to create world: Could not load " << bumpTextureFilePath << "!" << std::endl;
		return false;
	}

	// Load the voxel texture description file
	if(!LoadVoxelFaceTextureDesc(textureDescFilePath))
	{
		std::cerr << "Failed to create world: Could not load " << textureDescFilePath << "!" << std::endl;
		return false;
	}

	m_chunksInX = chunksInX;
	m_chunksInY = chunksInY;
	m_chunksInZ = chunksInZ;

	m_aabb = AABB(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(static_cast<float>(m_chunksInX * Chunk::s_chunkSizeX), static_cast<float>(m_chunksInY * Chunk::s_chunkSizeY), static_cast<float>(m_chunksInZ * Chunk::s_chunkSizeZ)));

	// Resize the chunk matrix
	m_chunkMatrix.resize(m_chunksInX);

	for(int x = 0; x < m_chunksInX; x++)
	{
		m_chunkMatrix[x].resize(m_chunksInY);

		for(int y = 0; y < m_chunksInY; y++)
		{
			m_chunkMatrix[x][y].resize(m_chunksInZ);

			for(int z = 0; z < m_chunksInZ; z++)
				m_chunkMatrix[x][y][z].Create(this, Point3i(x, y, z));
		}
	}

	float maxA;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxA);

	m_voxelTex_diffuse.GenMipMaps();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA);


	m_voxelTex_specular.GenMipMaps();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA);

	m_voxelTex_bump.GenMipMaps();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA);

	m_created = true;

	return true;
}

int World::GetChunksInX() const
{
	return m_chunksInX;
}

int World::GetChunksInY() const
{
	return m_chunksInY;
}

int World::GetChunksInZ() const
{
	return m_chunksInZ;
}

void World::OnAdd()
{
	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_pBatchRenderer = static_cast<Chunk_BatchRenderer*>(GetScene()->GetBatchRenderer("chunk", Chunk_BatchRenderer::Chunk_BatchRendererFactory));
	m_pBatchRenderer->m_pWorld = this;

	assert(m_pBatchRenderer != NULL);
}

bool World::LoadVoxelFaceTextureDesc(const std::string &fileName)
{
	assert(!m_created);

	std::ifstream fileIn(fileName);

	if(!fileIn.is_open())
		return false;

	// Load unique blocks. All others are assumed to have all the same sides
	std::vector<VoxelFaceDesc> uniqueDescs;

	// For marking which blocks are part of unique ones
	std::vector<bool> unique;

	Chunk::s_numTileTextures = Chunk::s_numTileTexturesInX * Chunk::s_numTileTexturesInY;

	unique.resize(Chunk::s_numTileTextures);

	// Default all to false
	for(unsigned int i = 0, size = unique.size(); i < size; i++)
		unique[i] = false;

	while(!fileIn.eof())
	{
		int blockID;

		fileIn >> blockID;

		unique[blockID - 1] = true;

		int top, bottom, xzSides;
		
		fileIn >> top >> bottom >> xzSides;

		// One less, since block IDs start at 1
		top--;
		bottom--;
		xzSides--;

		uniqueDescs.push_back(VoxelFaceDesc(top, bottom, xzSides));
	}

	int uniqueIndexCounter = 0;

	// Now that have unique ones, generate the full voxel description array for all (including not unique) voxel types
	for(int i = 0, size = unique.size(); i < size; i++)
	{
		if(unique[i])
		{
			// Unique block, add its custom data
			m_voxelTexturePosDescs.push_back(uniqueDescs[uniqueIndexCounter]);

			uniqueIndexCounter++;
		}
		else
		{
			// Not unique, give it default data
			m_voxelTexturePosDescs.push_back(VoxelFaceDesc(i, i, i));
		}
	}

	return true;
}

Point3i World::TransformPoint3i(const Point3i &point, int dir_xz)
{
	Point3i transformedPoint;

	switch(dir_xz)
	{
	case 1:
		transformedPoint.x = -point.z;
		transformedPoint.z = point.x;
		break;
	case 2:
		transformedPoint.x = -point.x;
		transformedPoint.z = -point.z;
		break;
	case 3:
		transformedPoint.x = point.z;
		transformedPoint.z = -point.x;
		break;
	}

	return transformedPoint;
}

unsigned char World::GetVoxel(int x, int y, int z)
{
	assert(m_created);

#ifdef OUTSIDE_CHUNKS_SOLID
	// If out of bounds, it is air
	if(x < 0 || x >= m_chunksInX * Chunk::s_chunkSizeX ||
		y < 0 || y >= m_chunksInY * Chunk::s_chunkSizeY ||
		z < 0 || z >= m_chunksInZ * Chunk::s_chunkSizeZ)
		return 1; // Random solid block
#else
	// If out of bounds, it is air
	if(x < 0 || x >= m_chunksInX * Chunk::s_chunkSizeX ||
		y < 0 || y >= m_chunksInY * Chunk::s_chunkSizeY ||
		z < 0 || z >= m_chunksInZ * Chunk::s_chunkSizeZ)
		return 0;
#endif

	int chunkX = x / Chunk::s_chunkSizeX;
	int chunkY = y / Chunk::s_chunkSizeY;
	int chunkZ = z / Chunk::s_chunkSizeZ;

	int localX = x % Chunk::s_chunkSizeX;
	int localY = y % Chunk::s_chunkSizeY;
	int localZ = z % Chunk::s_chunkSizeZ;

	return m_chunkMatrix[chunkX][chunkY][chunkZ].m_voxelMatrix[localX][localY][localZ];
}

void World::SetVoxel(int x, int y, int z, unsigned char type)
{
	assert(m_created);

	// If out of bounds, it is air
	if(x >= 0 && x < m_chunksInX * Chunk::s_chunkSizeX &&
		y >= 0 && y < m_chunksInY * Chunk::s_chunkSizeY &&
		z >= 0 && z < m_chunksInZ * Chunk::s_chunkSizeZ)
	{
		int chunkX = x / Chunk::s_chunkSizeX;
		int chunkY = y / Chunk::s_chunkSizeY;
		int chunkZ = z / Chunk::s_chunkSizeZ;

		if(type != 0)
			m_chunkMatrix[chunkX][chunkY][chunkZ].m_empty = false;

		int localX = x % Chunk::s_chunkSizeX;
		int localY = y % Chunk::s_chunkSizeY;
		int localZ = z % Chunk::s_chunkSizeZ;

		m_chunkMatrix[chunkX][chunkY][chunkZ].m_voxelMatrix[localX][localY][localZ] = type;
	}
}

void World::SetVoxel_NoCheck(int x, int y, int z, unsigned char type)
{
	assert(m_created);

	int chunkX = x / Chunk::s_chunkSizeX;
	int chunkY = y / Chunk::s_chunkSizeY;
	int chunkZ = z / Chunk::s_chunkSizeZ;

	int localX = x % Chunk::s_chunkSizeX;
	int localY = y % Chunk::s_chunkSizeY;
	int localZ = z % Chunk::s_chunkSizeZ;

	m_chunkMatrix[chunkX][chunkY][chunkZ].m_voxelMatrix[localX][localY][localZ] = type;
}

void World::IncVoxel(int x, int y, int z, char increment)
{
	assert(m_created);

	// If out of bounds, it is air
	assert(x >= 0 && x < m_chunksInX * Chunk::s_chunkSizeX &&
		y >= 0 && y < m_chunksInY * Chunk::s_chunkSizeY &&
		z >= 0 && z < m_chunksInZ * Chunk::s_chunkSizeZ);

	int chunkX = x / Chunk::s_chunkSizeX;
	int chunkY = y / Chunk::s_chunkSizeY;
	int chunkZ = z / Chunk::s_chunkSizeZ;

	int localX = x % Chunk::s_chunkSizeX;
	int localY = y % Chunk::s_chunkSizeY;
	int localZ = z % Chunk::s_chunkSizeZ;

	m_chunkMatrix[chunkX][chunkY][chunkZ].m_voxelMatrix[localX][localY][localZ] += increment;
}

void World::SetVoxelAndUpdate(const Vec3f &viewerDir, int x, int y, int z, unsigned char type)
{
	assert(m_created);

	// Change give coordinates based on id - if empty, delete selected block, otherwise put on the side
	if(type != 0)
	{
		x += static_cast<int>(viewerDir.x * 1.6f);
		y += static_cast<int>(viewerDir.y * 1.6f);
		z += static_cast<int>(viewerDir.z * 1.6f);
	}

	// Bounds check
	if(x < 0 || x >= m_chunksInX * Chunk::s_chunkSizeX ||
		y < 0 || y >= m_chunksInY * Chunk::s_chunkSizeY ||
		z < 0 || z >= m_chunksInZ * Chunk::s_chunkSizeZ)
		return;

	int chunkX = x / Chunk::s_chunkSizeX;
	int chunkY = y / Chunk::s_chunkSizeY;
	int chunkZ = z / Chunk::s_chunkSizeZ;

	int localX = x % Chunk::s_chunkSizeX;
	int localY = y % Chunk::s_chunkSizeY;
	int localZ = z % Chunk::s_chunkSizeZ;

	m_chunkMatrix[chunkX][chunkY][chunkZ].m_voxelMatrix[localX][localY][localZ] = type;

	// Update the chunk
	m_chunkMatrix[chunkX][chunkY][chunkZ].GenerateRenderAndPhysicsData();

	// Update surrounding chunks if voxel was at edge
	if(localX == 0)
	{
		int adjancentChunkX = chunkX - 1;

		if(adjancentChunkX >= 0)
			m_chunkMatrix[adjancentChunkX][chunkY][chunkZ].GenerateRenderAndPhysicsData();
	}
	else if(localX == Chunk::s_chunkSizeX - 1)
	{
		int adjancentChunkX = chunkX + 1;

		if(adjancentChunkX < m_chunksInX)
			m_chunkMatrix[adjancentChunkX][chunkY][chunkZ].GenerateRenderAndPhysicsData();
	}
	
	if(localY == 0)
	{
		int adjancentChunkY = chunkY - 1;

		if(adjancentChunkY >= 0)
			m_chunkMatrix[chunkX][adjancentChunkY][chunkZ].GenerateRenderAndPhysicsData();
	}
	else if(localY == Chunk::s_chunkSizeY - 1)
	{
		int adjancentChunkY = chunkY + 1;

		if(adjancentChunkY < m_chunksInY)
			m_chunkMatrix[chunkX][adjancentChunkY][chunkZ].GenerateRenderAndPhysicsData();
	}
	
	if(localZ == 0)
	{
		int adjancentChunkZ = chunkZ - 1;

		if(adjancentChunkZ >= 0)
			m_chunkMatrix[chunkX][chunkY][adjancentChunkZ].GenerateRenderAndPhysicsData();
	}
	else if(localZ == Chunk::s_chunkSizeZ - 1)
	{
		int adjancentChunkZ = chunkZ + 1;

		if(adjancentChunkZ < m_chunksInZ)
			m_chunkMatrix[chunkX][chunkY][adjancentChunkZ].GenerateRenderAndPhysicsData();
	}
}

Point3i World::GetRayCollisionDist(const Vec3f &start, const Vec3f &dir, float castDist) // Returns -1.0f of the ray didn't hit anything
{
	float dist = 1.6f;

	// Test every block length, so will hit all blocks no matter what
	while(dist <= castDist)
	{
		// Get the position along the ray
		Vec3f pos(start + dir * dist);

		// Get the voxel at this position
		int vx = static_cast<int>(pos.x);
		int vy = static_cast<int>(pos.y);
		int vz = static_cast<int>(pos.z);

		unsigned char type = GetVoxel(vx, vy, vz);

		// If solid, stop testing
		if(type != 0)
			return Point3i(vx, vy, vz);

		dist += 1.0f;
	}

	// Didn't hit anything, return may power
	return Point3i(-1, -1, -1);
}

Point3i World::HighlightVoxel(const Vec3f &viewerPos, const Vec3f &viewerDir)
{
	return GetRayCollisionDist(viewerPos, viewerDir, 30.0f);
}

void World::SetMaterial()
{
	GetScene()->SetCurrentGBufferRenderShader(Scene::e_bump);

	m_voxelTex_diffuse.Bind();

	GetScene()->UseSpecularTexture(true);
	GetScene()->SetSpecularColor(1.0f);

	glActiveTexture(GL_TEXTURE3);

	m_voxelTex_bump.Bind();

	glActiveTexture(GL_TEXTURE1);
	
	m_voxelTex_specular.Bind();

	glActiveTexture(GL_TEXTURE0);
}

void World::ResetMaterial()
{
	GetScene()->SetCurrentGBufferRenderShader(Scene::e_plain);

	GetScene()->UseSpecularTexture(false);
	GetScene()->SetSpecularColor(0.0f);
}

void World::RenderAllChunks()
{
	viewerPosition = GetScene()->m_camera.m_position;

	// Enable arrays
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	for(int x = 0; x < m_chunksInX; x++)
		for(int y = 0; y < m_chunksInY; y++)
			for(int z = 0; z < m_chunksInZ; z++)
				if(!m_chunkMatrix[x][y][z].IsEmpty())
					m_chunkMatrix[x][y][z].Draw();

	VBO::Unbind(GL_ARRAY_BUFFER);

	// Disable arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void World::GenerateAllChunks()
{
	// Generate chunk render data
	for(int x = 0; x < m_chunksInX; x++)
		for(int y = 0; y < m_chunksInY; y++)
			for(int z = 0; z < m_chunksInZ; z++)
				m_chunkMatrix[x][y][z].GenerateRenderAndPhysicsData();
}

void World::FillBox(const Point3i &lowerBound, const Point3i &upperBound, unsigned char id)
{
	for(int x = lowerBound.x; x < upperBound.x; x++)
		for(int y = lowerBound.y; y < upperBound.y; y++)
			for(int z = lowerBound.z; z < upperBound.z; z++)
				SetVoxel(x, y, z, id);
}

void World::StartPathFindingTask(PathFindingTask* pTask)
{
	assert(pTask != NULL);

	// Only add the task of there are open threads in order to prevent task overflow
	if(m_pathFindingThreadPool.HasOpenThreads())
	{
		pTask->m_taskDone = false;
		pTask->m_pWorld = this;
		pTask->m_path.clear();

		m_pathFindingThreadPool.AddTask(pTask);
	}
}

bool DistCompare(OctreeOccupant* first, OctreeOccupant* second)
{
	// Compare distance to center. Can use magnitude squared, since comparing, and if magnitude squared is greater, magnitude would be greater as well
	return ((viewerPosition - first->GetAABB().GetCenter()).MagnitudeSquared()) < ((viewerPosition - second->GetAABB().GetCenter()).MagnitudeSquared()) ? true : false;
}