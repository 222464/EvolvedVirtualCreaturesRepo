#pragma once

#include <Renderer/Octree/Octree.h>

class StaticOctree :
	public Octree
{
private:
	bool m_created;

public:
	StaticOctree();
	StaticOctree(const AABB &rootRegion);

	void Create(const AABB &rootRegion);

	// Inherited from Octree
	void Add(OctreeOccupant* pOc);

	void Clear();
	void Clear(const AABB &rootRegion);

	AABB GetRootAABB();
};

