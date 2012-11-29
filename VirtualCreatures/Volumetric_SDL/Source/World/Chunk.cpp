#include <World/Chunk.h>

#include <World/World.h>

#include <Utilities/UtilFuncs.h>

#include <vector>

#include <assert.h>

#include <iostream>

// Defaults
unsigned char Chunk::s_numTileTexturesInX = 2;
unsigned char Chunk::s_numTileTexturesInY = 4;

unsigned char Chunk::s_numTileTextures = 8;

float Chunk::s_friction = 0.5f;

const float occlusionMultiplier = 3.0f;

Chunk::Chunk()
	: m_created(false), m_empty(true), m_physicsCreated(false),
	m_pChunkRenderer(NULL)
{
}

Chunk::~Chunk()
{
	// Delete physics
	if(m_physicsCreated)
	{
		delete m_pTriangleMesh;
		delete m_pMeshShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

void Chunk::Create(World* pWorld, const Point3i &matrixPos)
{
	m_pWorld = pWorld;
	m_matrixPos = matrixPos;
	m_worldPos.x = static_cast<float>(m_matrixPos.x * s_chunkSizeX);
	m_worldPos.y = static_cast<float>(m_matrixPos.y * s_chunkSizeY);
	m_worldPos.z = static_cast<float>(m_matrixPos.z * s_chunkSizeZ);
	m_empty = true;

	// Set up AABB
	m_aabb.m_lowerBound.x = 0.0f;
	m_aabb.m_lowerBound.y = 0.0f;
	m_aabb.m_lowerBound.z = 0.0f;

	m_aabb.m_upperBound.x = static_cast<float>(s_chunkSizeX);
	m_aabb.m_upperBound.y = static_cast<float>(s_chunkSizeY);
	m_aabb.m_upperBound.z = static_cast<float>(s_chunkSizeZ);

	// Must calculate AABB attributes in this order
	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	m_aabb.IncCenter(Vec3f(matrixPos.x * static_cast<float>(s_chunkSizeX), matrixPos.y * static_cast<float>(s_chunkSizeY), matrixPos.z * static_cast<float>(s_chunkSizeZ)));

	m_created = true;
}

void Chunk::AddGeometry(const Point3i &lower, int side, std::vector<Vec3f> &vertexArray, char voxelType)
{
	assert(m_created);

	// Possible indices - lower and 1 past lower in all directions (upper)
	// Lower equates to the chunk matrix positions
	Point3i upper(lower);
	upper.x++;
	upper.y++;
	upper.z++;

	Vec3f lowerf(static_cast<float>(lower.x), static_cast<float>(lower.y), static_cast<float>(lower.z));
	Vec3f upperf(static_cast<float>(upper.x), static_cast<float>(upper.y), static_cast<float>(upper.z));

	// Surrounding indices
	switch(side)
	{
	case 0:
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, upperf.z) + m_worldPos);

		break;
	case 1:
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, lowerf.z) + m_worldPos);

		break;
	case 2:
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, lowerf.z) + m_worldPos);

		break;
	case 3:
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, upperf.z) + m_worldPos);

		break;
	case 4:
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, upperf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, upperf.z) + m_worldPos);

		break;
	case 5:
		vertexArray.push_back(Vec3f(upperf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, lowerf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(lowerf.x, upperf.y, lowerf.z) + m_worldPos);
		vertexArray.push_back(Vec3f(upperf.x, upperf.y, lowerf.z) + m_worldPos);

		break;
	}
}

int Chunk::GetSideFromDir(const Point3i &dir)
{
	if(dir.x != 0)
	{
		if(dir.x > 0)
			return 0;
		else
			return 1;
	}
	else if(dir.y != 0)
	{
		if(dir.y > 0)
			return 2;
		else
			return 3;
	}
	else
	{
		if(dir.z > 0)
			return 4;
		else
			return 5;
	}
}

void Chunk::AddTexCoord(std::vector<Vec2f> &texCoordArray, unsigned char id, int side)
{
	assert(m_created);

	// Get ID
	int voxelDescID = id - 1;

	m_pWorld->m_voxelTexturePosDescs[voxelDescID].AddCoords(side, texCoordArray);
}

void Chunk::AddNormals(std::vector<Vec3f> &normalArray, int side)
{
	for(int c = 0; c < 4; c++)
		normalArray.push_back(m_pWorld->m_normals[side]);
}

void Chunk::AddColor(Color3f occlusionFactor[4], int side, std::vector<Color3f> &colorArray)
{
	for(unsigned int i = 0; i < 4; i++)
		colorArray.push_back(occlusionFactor[i]);
}

void Chunk::GetSemiSphereOcclusion(const Vec3f &blockPos, int side, Color3f data[4])
{
	// 4 corners
	/*for(int c = 0; c < 4; c++)
	{
		float escapedCosineSum = 0.0f;
		float cosineSum = 0.0f;

		for(int pi = 0; pi < m_pWorld->s_numAOSamples; pi++)
		{
			Vec3f dir(m_pWorld->m_sphereDistribution[pi]);

			dir = dir.Normalize();

			float lambert = dir.Dot(m_pWorld->m_normals[side]);

			if(lambert <= 0.0f)
				continue;

			Vec3f rayStart(blockPos + m_pWorld->m_corners[side][c]);

			// Offset to the face side
			float colDist = GetRayCollisionDist(rayStart, dir, m_pWorld->m_AOSampleDistance);

			// If hit nothing
			if(colDist == -1.0f)
			{
				// Multiply this value by the angle, so beams that are perpendicular to surface are worth more (lambertarian lighting)
				escapedCosineSum += lambert;
				cosineSum += lambert;
			}
			else
				// Multiply this value by the angle, so beams that are perpendicular to surface are worth more (lambertarian lighting)
				cosineSum += lambert; //  * (colDist / s_ambientOcclusionSampleDistance)
		}

		float term = escapedCosineSum / cosineSum;
		data[c] = Color3f(term, term, term);
	}*/

	for(int c = 0; c < 4; c++)
	{
		Vec3f rayStart(blockPos + m_pWorld->m_corners[side][c]);

		// Count blocks surrounding the corner
		float occlusion = 4.0f;

		for(float x = 0; x < 2; x++)
			for(float y = 0; y < 2; y++)
				for(float z = 0; z < 2; z++)
				{
					Vec3f posf(rayStart + Vec3f(x - 0.5f, y - 0.5f, z - 0.5f));

					if(m_pWorld->GetVoxel(static_cast<int>(posf.x), static_cast<int>(posf.y), static_cast<int>(posf.z)) != 0)
						occlusion--;
				}

		float term = occlusion / 4.0f;
		data[c] = Color3f(term, term, term);
	}
}

unsigned short Chunk::PositionToIndex(const Point3i &position)
{
	return static_cast<unsigned short>(position.z + position.y * s_onePastChunkSizeZ + position.x * s_onePastChunkSizeY * s_onePastChunkSizeZ);
}

float Chunk::GetRayCollisionDist(const Vec3f &start, const Vec3f &dir, float maxDist)
{
	int numIgnored = 0;

	float dist = 0.7071067f;

	// Test every block length, so will hit all blocks no matter what
	while(dist <= maxDist)
	{
		// Get the position along the ray
		Vec3f pos(start + dir * dist);

		// Get the voxel at this position
		int vx = static_cast<int>(pos.x);
		int vy = static_cast<int>(pos.y);
		int vz = static_cast<int>(pos.z);

		unsigned char type = m_pWorld->GetVoxel(vx, vy, vz);

		// If solid, stop testing
		if(type != 0)
			return dist;

		dist += 1.0f;
	}

	// Didn't hit anything, return may power
	return -1.0f;
}

void Chunk::GenerateRenderAndPhysicsData()
{
	assert(m_created);
	assert(m_pWorld->m_pPhysicsWorld != NULL);

	if(m_empty)
		return;

	// Set empty to true, will remain unless set false when comes across full voxel in generation
	m_empty = true;

	std::vector<Vec3f> vertexArray;
	std::vector<Vec2f> texCoordArray;
	std::vector<Color3f> colorArray;
	std::vector<Vec3f> normalArray;

	// Go through voxels (not differentiating between border voxels and inside voxels)
	for(int x = 0; x < s_chunkSizeX; x++)
		for(int y = 0; y < s_chunkSizeY; y++)
			for(int z = 0; z < s_chunkSizeZ; z++)
			{
				// If voxel is not empty
				if(m_voxelMatrix[x][y][z] == 0 || m_voxelMatrix[x][y][z] > s_numTileTextures)
					continue;

				// Check surrounding voxels to see if they are empty
				for(int side = 0; side < 6; side++)
				{
					Point3i offset(m_pWorld->m_positionOffsets[side]);

					int new_x = x + offset.x;
					int new_y = y + offset.y;
					int new_z = z + offset.z;

					unsigned char voxelType;

					// If check location is outside this chunk, get the neighboring chunk
					if(new_x < 0 || new_x >= s_chunkSizeX ||
						new_y < 0 || new_y >= s_chunkSizeY ||
						new_z < 0 || new_z >= s_chunkSizeZ)
						voxelType = m_pWorld->GetVoxel(m_matrixPos.x * s_chunkSizeX + new_x, m_matrixPos.y * s_chunkSizeY + new_y, m_matrixPos.z * s_chunkSizeZ + new_z);
					else
						voxelType = m_voxelMatrix[new_x][new_y][new_z];

					// If empty
					if(voxelType == 0 || voxelType > s_numTileTextures)
					{
						AddGeometry(Point3i(x, y, z), side, vertexArray, voxelType);

						Vec3f blockPosf(static_cast<float>(m_matrixPos.x * s_chunkSizeX + new_x),
							static_cast<float>(m_matrixPos.y * s_chunkSizeY + new_y),
							static_cast<float>(m_matrixPos.z * s_chunkSizeZ + new_z));

						AddTexCoord(texCoordArray, m_voxelMatrix[x][y][z], side);

						Color3f data[4];

						GetSemiSphereOcclusion(blockPosf, side, data);

						AddColor(data, side, colorArray);

						AddNormals(normalArray, side);
					}
				}
			}

	m_numVertices = vertexArray.size();

	m_empty = m_numVertices == 0;

	// Destroy any existing buffers
	/*if(m_vertexBuffer.Created())
		m_vertexBuffer.Destroy();

	if(m_texCoordBuffer.Created())
		m_texCoordBuffer.Destroy();

	if(m_colorBuffer.Created())
		m_colorBuffer.Destroy();

	if(m_normalBuffer.Created())
		m_normalBuffer.Destroy();*/

	if(m_empty)
	{
		if(m_pChunkRenderer != NULL)
		{
			m_pChunkRenderer->Destroy();

			m_pChunkRenderer = NULL;
		}

		return;
	}
	
	// Create (or re-create) VBOs
	if(!m_vertexBuffer.Created())
		m_vertexBuffer.Create();

	if(!m_texCoordBuffer.Created())
		m_texCoordBuffer.Create();

	if(!m_colorBuffer.Created())
		m_colorBuffer.Create();

	if(!m_normalBuffer.Created())
		m_normalBuffer.Create();

	// Vertex VBO
	m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_numVertices, &vertexArray[0], GL_STATIC_DRAW);
	m_vertexBuffer.Unbind();

	// TexCoord VBO
	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * m_numVertices, &texCoordArray[0], GL_STATIC_DRAW);
	m_texCoordBuffer.Unbind();

	// Color VBO
	m_colorBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_numVertices, &colorArray[0], GL_STATIC_DRAW);
	m_colorBuffer.Unbind();

	// Normal VBO
	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_numVertices, &normalArray[0], GL_STATIC_DRAW);
	m_normalBuffer.Unbind();

	GL_ERROR_CHECK();

	if(m_pChunkRenderer == NULL)
	{
		m_pChunkRenderer = new Chunk_Renderer(this, m_aabb);
		m_pWorld->GetScene()->Add(m_pChunkRenderer, true);
	}

	// ---------------------------------- Physics ----------------------------------

	// If body already exists, remove it and delete
	if(m_physicsCreated)
	{
		m_pWorld->m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pTriangleMesh;
		delete m_pMeshShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}

	m_pTriangleMesh = new btTriangleMesh();

	// Add all vertices
	for(unsigned int i = 0; i < m_numVertices; i += 4)
	{
		// Triangulate quad
		const Vec3f &vertex0 = vertexArray[i];
		const Vec3f &vertex1 = vertexArray[i + 1];
		const Vec3f &vertex2 = vertexArray[i + 2];
		const Vec3f &vertex3 = vertexArray[i + 3];

		m_pTriangleMesh->addTriangle(bt(vertex0), bt(vertex1), bt(vertex2), false);

		m_pTriangleMesh->addTriangle(bt(vertex0), bt(vertex2), bt(vertex3), false);
	}

	m_pMeshShape = new btBvhTriangleMeshShape(m_pTriangleMesh, true, true);

	m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 0.0f, 0.0f)));

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, m_pMotionState, m_pMeshShape, btVector3(0.0f, 0.0f, 0.0f));

	rigidBodyCI.m_restitution = 0.0f;
	rigidBodyCI.m_friction = s_friction;

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	m_pWorld->m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	m_pRigidBody->setUserPointer(m_pWorld);

	m_physicsCreated = true;

	GL_ERROR_CHECK();
}

void Chunk::Draw()
{
	assert(m_created);
	assert(!m_empty);

	// Bind the vertex buffer
	m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL); // NULL tells OpenGL to use the last bound buffer

	// Bind the tex coord buffer
	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL); // NULL tells OpenGL to use the last bound buffer

	// Bind the color buffer
	m_colorBuffer.Bind(GL_ARRAY_BUFFER);
	glColorPointer(3, GL_FLOAT, 0, NULL); // NULL tells OpenGL to use the last bound buffer

	// Bind the normal buffer
	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL); // NULL tells OpenGL to use the last bound buffer

	// Visible, draw list
	glDrawArrays(GL_QUADS, 0, m_numVertices); // NULL at end makes it use the bound VBO

	GL_ERROR_CHECK();
}

bool Chunk::IsEmpty()
{
	return m_empty;
}

Chunk_Renderer::Chunk_Renderer(Chunk* pChunk, const AABB &aabb)
	: m_pChunk(pChunk)
{
	m_aabb = aabb;
}

void Chunk_Renderer::Render()
{
	m_pChunk->m_pWorld->m_pBatchRenderer->Add(this);
}

Chunk_BatchRenderer::Chunk_BatchRenderer()
	: m_pWorld(NULL)
{
}

void Chunk_BatchRenderer::Add(Chunk_Renderer* pChunkRenderer)
{
	m_chunks.push_back(pChunkRenderer);
}

void Chunk_BatchRenderer::Execute()
{
	assert(m_pWorld != NULL);

	m_pWorld->SetMaterial();

	// Enable arrays
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_COLOR_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	for(std::list<Chunk_Renderer*>::iterator it = m_chunks.begin(); it != m_chunks.end(); it++)
		(*it)->m_pChunk->Draw();

	// Reset
	m_pWorld->ResetMaterial();

	VBO::Unbind(GL_ARRAY_BUFFER);

	// Disable arrays
	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_COLOR_ARRAY);
	glDisable(GL_NORMAL_ARRAY);
}

void Chunk_BatchRenderer::Clear()
{
	m_chunks.clear();
}

BatchRenderer* Chunk_BatchRenderer::Chunk_BatchRendererFactory()
{
	return new Chunk_BatchRenderer();
}