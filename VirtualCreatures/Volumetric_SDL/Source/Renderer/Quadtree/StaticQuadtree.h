#pragma once

#include <Renderer/Quadtree/Quadtree.h>

class StaticQuadtree :
	public Quadtree
{
private:
	bool m_created;

public:
	StaticQuadtree();
	StaticQuadtree(const AABB2D &rootRegion);

	void Create(const AABB2D &rootRegion);

	// Inherited from Quadtree
	void Add(QuadtreeOccupant* pOc);

	void Clear();
	void Clear(const AABB2D &rootRegion);

	AABB2D GetRootAABB2D();
};

