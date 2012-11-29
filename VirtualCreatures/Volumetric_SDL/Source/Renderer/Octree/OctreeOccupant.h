#pragma once

#include <Constructs/AABB.h>

class Octree;
class OctreeNode;

class OctreeOccupant
{
private:
	OctreeNode* m_pOctreeNode;
	Octree* m_pOctree;

protected:
	AABB m_aabb;

public:
	OctreeOccupant();
	virtual ~OctreeOccupant() {}

	void TreeUpdate();
	void RemoveFromTree();

	const AABB &GetAABB();

	Octree* GetTree();

	friend class OctreeNode;
	friend class Octree;
};

