#include <SceneObjects/Physics/DynamicConcaveShape.h>

#include <Constructs/Vec3f.h>
#include <Constructs/BulletConversions.h>

#include <fstream>
#include <iostream>

#include <assert.h>

DynamicConcaveShape::DynamicConcaveShape()
	: m_loaded(false)
{
}

DynamicConcaveShape::~DynamicConcaveShape()
{
	for(int c = 0, numClusters = m_pConvexMeshes.size(); c < numClusters; c++)
		delete m_pConvexMeshes[c];
}

// Inherited from the asset manager
bool DynamicConcaveShape::LoadAsset(const std::string &name)
{
	assert(!m_loaded);

	std::ifstream fromFile(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not open file \"" << name << "\"!" << std::endl;

		return false;
	}

	std::string param;

	// Load version
	fromFile >> param;

	float version;

	fromFile >> version;

	if(version != 1.0f)
	{
		std::cerr << "Version mismatch in file \"" << name << "\"!" << std::endl;

		fromFile.close();

		return false;
	}

	// Load number of clusters
	fromFile >> param;

	int numClusters;

	fromFile >> numClusters;

	// Load clusters
	for(int c = 0; c < numClusters; c++)
	{
		// Read cluster header
		fromFile >> param >> param;

		// Read number of points
		fromFile >> param;
		
		int numPoints;

		fromFile >> numPoints;

		// Read number of triangles
		fromFile >> param;
		
		int numTriangles;

		fromFile >> numTriangles;

		btConvexHullShape* pConvexShape = new btConvexHullShape();

		m_pConvexMeshes.push_back(pConvexShape);

		// Read points header
		fromFile >> param >> param;

		for(int p = 0; p < numPoints; p++)
		{
			Vec3f point;

			fromFile >> point.x >> point.y >> point.z;

			pConvexShape->addPoint(bt(point));
		}

		// Read points ender
		fromFile >> param;

		// Read triangles header
		fromFile >> param >> param;

		// btConvexHull doesn't use indices (but other shapes do), so in this case we ignore the vertices.
		int parami;

		for(int t = 0; t < numTriangles; t++)
			fromFile >> parami >> parami >> parami;

		// Read triangles ender
		fromFile >> param;

		// Read cluster ender
		fromFile >> param;
	}

	assert(numClusters == m_pConvexMeshes.size());

	btTransform trans;

	trans.setIdentity();

	// Add all meshes to compound shape
	for(int c = 0; c < numClusters; c++)
		m_compoundShape.addChildShape(trans, m_pConvexMeshes[c]);

	return true;
}

btCompoundShape* DynamicConcaveShape::GetShape()
{
	return &m_compoundShape;
}

unsigned int DynamicConcaveShape::GetNumConvexMeshes() const
{
	return m_pConvexMeshes.size();
}

btConvexHullShape* DynamicConcaveShape::GetConvexMesh(unsigned int index)
{
	return m_pConvexMeshes[index];
}

Asset* DynamicConcaveShape::Asset_Factory()
{
	return new DynamicConcaveShape();
}