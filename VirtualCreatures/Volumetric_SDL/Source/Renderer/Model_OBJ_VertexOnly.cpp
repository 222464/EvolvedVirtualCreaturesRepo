#include <Renderer/Model_OBJ_VertexOnly.h>

#include <Constructs/Vec3f.h>
#include <Constructs/Vec2f.h>

#include <fstream>
#include <sstream>

#include <vector>

#include <unordered_map>

#include <iostream>

#include <assert.h>

Model_OBJ_VertexOnly::Model_OBJ_VertexOnly()
	: m_loaded(false)
{
}

bool Model_OBJ_VertexOnly::LoadAsset(const std::string &name)
{
	assert(!m_loaded);

	std::ifstream fromFile(name);
	
	if(!fromFile.is_open())
	{
		std::cerr << "Could not load model file " << name << std::endl;
		return false;
	}

	std::vector<Vec3f> fileVertices;

	std::vector<Vec3f> vertices;
	std::vector<unsigned short> indices;

	// Hash map for linking indices to vertex array index for attributes
	std::unordered_map<unsigned int, unsigned int> indexToVertex;

	// Initial extremes
	m_aabb.m_lowerBound = Vec3f(9999.0f, 9999.0f, 9999.0f);
	m_aabb.m_upperBound = Vec3f(-9999.0f, -9999.0f, -9999.0f);

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

			fileVertices.push_back(Vec3f(x, y, z));

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
		else if(header == "f")
		{
			// Add a face
			unsigned int v[3];

			ss >> v[0];
			ss >> v[1];
			ss >> v[2];

			for(int i = 0; i < 3; i++)
			{
				// Search for index set 1
				std::unordered_map<unsigned int, unsigned int>::iterator it = indexToVertex.find(v[i]);

				if(it == indexToVertex.end())
				{
					// Vertex attributes do not exist, create them

					// File indicies start at 1, so convert
					unsigned int vertIndex = v[i] - 1;

					vertices.push_back(fileVertices[vertIndex]);

					// Index of vertex in vertex component array
					unsigned int realIndex = vertices.size() - 1;

					// Add attribute set index to the map
					indexToVertex[v[i]] = realIndex;

					indices.push_back(static_cast<unsigned short>(realIndex));
				}
				else
				{
					// Index already exists, so add it
					indices.push_back(static_cast<unsigned short>(it->second));
				}
			}
		}
	}

	fromFile.close();

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	m_numVertices = indices.size();

	// Create the VBOs
	m_vertices.Create();
	m_vertices.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	m_vertices.Unbind();

	m_indices.Create();
	m_indices.Bind(GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices.size(), &indices[0], GL_STATIC_DRAW);
	m_indices.Unbind();

	m_loaded = true;

	GL_ERROR_CHECK();

	return true;
}

void Model_OBJ_VertexOnly::Render()
{
	assert(m_loaded);

	m_vertices.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m_indices.Bind(GL_ELEMENT_ARRAY_BUFFER);

	glDrawElements(GL_TRIANGLES, m_numVertices, GL_UNSIGNED_SHORT, NULL);

	m_indices.Unbind();
	m_vertices.Unbind();

	GL_ERROR_CHECK();
}

bool Model_OBJ_VertexOnly::Loaded()
{
	return m_loaded;
}

AABB Model_OBJ_VertexOnly::GetAABB()
{
	return m_aabb;
}

const Vec3f &Model_OBJ_VertexOnly::GetAABBOffsetFromModel()
{
	return m_aabb.GetCenter();
}

Asset* Model_OBJ_VertexOnly::Asset_Factory()
{
	return new Model_OBJ_VertexOnly();
}