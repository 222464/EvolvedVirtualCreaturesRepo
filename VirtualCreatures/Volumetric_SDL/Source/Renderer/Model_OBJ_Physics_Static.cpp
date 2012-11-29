#include <Renderer/Model_OBJ_Physics_Static.h>

#include <Constructs/Vec3f.h>
#include <Constructs/Vec2f.h>

#include <Utilities/UtilFuncs.h>

#include <fstream>
#include <sstream>

#include <vector>

#include <unordered_map>

#include <iostream>

#include <assert.h>

Model_OBJ_Physics_Static::~Model_OBJ_Physics_Static()
{
	// Delete physics
	if(m_loaded)
	{
		delete m_pMesh;
		delete m_pMeshShape;
	}
}

btBvhTriangleMeshShape* Model_OBJ_Physics_Static::GetShape() const
{
	return m_pMeshShape;
}

bool Model_OBJ_Physics_Static::LoadAsset(const std::string &name)
{
	assert(!m_loaded);

	std::ifstream fromFile(name);
	
	if(!fromFile.is_open())
	{
		std::cerr << "Could not load model file " << name << std::endl;
		return false;
	}

	std::string rootName(GetRootName(name));

	std::vector<Vec3f> filePositions;
	std::vector<Vec2f> fileTexCoords;
	std::vector<Vec3f> fileNormals;

	std::vector<Vec2f> texCoords;
	std::vector<Vec3f> normals;

	// Hash map for linking indices to vertex array index for attributes
	std::unordered_map<IndexSet, unsigned int, IndexSet> indexToVertex;

	// Initial extremes
	m_aabb.m_lowerBound = Vec3f(9999.0f, 9999.0f, 9999.0f);
	m_aabb.m_upperBound = Vec3f(-9999.0f, -9999.0f, -9999.0f);

	int currentObj = -1;

	std::unordered_map<std::string, unsigned int> matReferences;

	while(!fromFile.eof())
	{
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if(header == "v")
		{
			// Add vertex
			float x, y, z;

			ss >> x >> y >> z;

			filePositions.push_back(Vec3f(x, y, z));

			// Expand AABB
			if(x < m_aabb.m_lowerBound.x)
				m_aabb.m_lowerBound.x = x;
			if(y < m_aabb.m_lowerBound.y)
				m_aabb.m_lowerBound.y = y;
			if(z < m_aabb.m_lowerBound.z)
				m_aabb.m_lowerBound.z = z;

			if(x > m_aabb.m_upperBound.x)
				m_aabb.m_upperBound.x = x;
			if(y > m_aabb.m_upperBound.y)
				m_aabb.m_upperBound.y = y;
			if(z > m_aabb.m_upperBound.z)
				m_aabb.m_upperBound.z = z;
		}
		else if(header == "vt")
		{
			// Add texture coordinate
			float s, t;

			ss >> s >> t;

			fileTexCoords.push_back(Vec2f(s, t));
		}
		else if(header == "vn")
		{
			// Add normal
			float nx, ny, nz;

			ss >> nx >> ny >> nz;

			fileNormals.push_back(Vec3f(nx, ny, nz));
		}
		else if(header == "f")
		{
			assert(m_indices.size() > 0);
			assert(currentObj == m_indices.size() - 1);

			// Add a face
			IndexSet v[3];

			ss >> v[0].vi;
			ss.ignore(1, '/');
			ss >> v[0].ti;
			ss.ignore(1, '/');
			ss >> v[0].ni;

			ss >> v[1].vi;
			ss.ignore(1, '/');
			ss >> v[1].ti;
			ss.ignore(1, '/');
			ss >> v[1].ni;

			ss >> v[2].vi;
			ss.ignore(1, '/');
			ss >> v[2].ti;
			ss.ignore(1, '/');
			ss >> v[2].ni;

			for(int i = 0; i < 3; i++)
			{
				// Search for index set 1
				std::unordered_map<IndexSet, unsigned int>::iterator it = indexToVertex.find(v[i]);

				if(it == indexToVertex.end())
				{
					// Vertex attributes do not exist, create them

					// File indicies start at 1, so convert
					unsigned int vertIndex = v[i].vi - 1;
					unsigned int texCoordIndex = v[i].ti - 1;
					unsigned int normalIndex = v[i].ni - 1;

					m_positions.push_back(filePositions[vertIndex]);
					texCoords.push_back(fileTexCoords[texCoordIndex]);
					normals.push_back(fileNormals[normalIndex]);

					// Index of vertex in vertex component array
					unsigned int realIndex = m_positions.size() - 1;

					// Add attribute set index to the map
					indexToVertex[v[i]] = realIndex;

					m_indices[currentObj].push_back(static_cast<unsigned short>(realIndex));
				}
				else
				{
					// Index already exists, so add it
					m_indices[currentObj].push_back(static_cast<unsigned short>(it->second));
				}
			}
		}
		else if(header == "usemtl")
		{
			if(m_usingMTL)
			{
				// Get texture name and load it
				std::string matName;
				ss >> matName;

				// Link obj to material
				std::unordered_map<std::string, unsigned int>::iterator it = matReferences.find(matName);

				if(it == matReferences.end())
				{
					std::cerr << "Could not find material \"" << matName << "\"!" << std::endl;
					return false;
				}

				m_objMaterialReferences.push_back(it->second);
			}
			else
			{
				// Get texture name and load it
				std::string texName;
				ss >> texName;

				// Add tex name on to normal name (relative to model file name)
				std::stringstream fullTexName;
				fullTexName << rootName << texName;

				// New OBJ object texture
				Asset* pAsset;

				if(!m_textureManager.GetAsset(fullTexName.str(), pAsset))
					return false;

				Asset_Texture* pTexture = static_cast<Asset_Texture*>(pAsset);

				Material mat;
				mat.m_pDiffuseMap = pTexture;
				mat.m_shader = Scene::e_plain;

				m_materials.push_back(mat);
				m_objMaterialReferences.push_back(m_materials.size() - 1);

				pTexture->GenMipMaps();
			}

			// Next index in indices array
			m_indices.push_back(std::vector<unsigned short>());
			currentObj++;
		}
		else if(!m_usingMTL && header == "mtllib")
		{
			// Using a material library. Load the materials
			m_usingMTL = true;

			std::string libName;
			ss >> libName;

			std::ostringstream fullMaterialLibraryName;

			fullMaterialLibraryName << rootName << libName;

			if(!LoadMaterialLibrary(fullMaterialLibraryName.str(), matReferences))
			{
				std::cerr << "- in " << name << std::endl;

				return false;
			}
		}
	}

	fromFile.close();

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	// Create the VBOs
	m_positionBuffer.Create();
	m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * m_positions.size(), &m_positions[0], GL_STATIC_DRAW);
	m_positionBuffer.Unbind();

	m_texCoordBuffer.Create();
	m_texCoordBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
	m_texCoordBuffer.Unbind();

	m_normalBuffer.Create();
	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	m_normalBuffer.Unbind();

	const unsigned int numObjects = m_indices.size();

	// Create index VBOs
	m_numVertices.resize(numObjects);
	m_indexBuffers.resize(numObjects);

	for(unsigned int i = 0; i < numObjects; i++)
	{
		m_numVertices[i] = m_indices[i].size();

		m_indexBuffers[i].Create();
		m_indexBuffers[i].Bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * m_numVertices[i], &m_indices[i][0], GL_STATIC_DRAW);
		m_indexBuffers[i].Unbind();
	}

	FindShaderCombination();

	// ----------------------------------------- Physics -----------------------------------------

	m_pMesh = new btTriangleIndexVertexArray();

	for(unsigned int i = 0; i < numObjects; i++)
	{
		btIndexedMesh newMesh;

		newMesh.m_numTriangles = m_numVertices[i] / 3;
		newMesh.m_numVertices = m_numVertices[i];

		newMesh.m_triangleIndexStride = sizeof(unsigned short) * 3;
		newMesh.m_triangleIndexBase = (const unsigned char*)(&m_indices[i][0]);

		newMesh.m_vertexStride = sizeof(float) * 3;
		newMesh.m_vertexBase = (const unsigned char*)(&m_positions[0]);

		m_pMesh->addIndexedMesh(newMesh, PHY_SHORT);
	}

	m_pMeshShape = new btBvhTriangleMeshShape(m_pMesh, true, true);

	m_loaded = true;

	GL_ERROR_CHECK();

	return true;
}

Asset* Model_OBJ_Physics_Static::Asset_Factory()
{
	return new Model_OBJ_Physics_Static();
}