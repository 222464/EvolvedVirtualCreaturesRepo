#pragma once

#include <Constructs/AABB2D.h>

class Quadtree;
class QuadtreeNode;

class QuadtreeOccupant
{
private:
	QuadtreeNode* m_pQuadtreeNode;
	Quadtree* m_pQuadtree;

protected:
	AABB2D m_aabb2D;

public:
	QuadtreeOccupant();
	virtual ~QuadtreeOccupant() {}

	void TreeUpdate();
	void RemoveFromTree();

	const AABB2D &GetAABB2D();

	Quadtree* GetTree();

	friend class QuadtreeNode;
	friend class Quadtree;
};

