#pragma once

#include <Renderer/Model_OBJ.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

class Model_OBJ_Physics_Static :
	public Model_OBJ
{
private:
	std::vector<Vec3f> m_positions;
	std::vector<std::vector<unsigned short>> m_indices;

	// Physics
	btTriangleIndexVertexArray* m_pMesh;
	btBvhTriangleMeshShape* m_pMeshShape;

public:
	~Model_OBJ_Physics_Static();

	btBvhTriangleMeshShape* GetShape() const;

	// Inherited from Model_OBJ_Physics_Static/Asset
	bool LoadAsset(const std::string &name);

	// Asset factory
	static Asset* Asset_Factory();
};

