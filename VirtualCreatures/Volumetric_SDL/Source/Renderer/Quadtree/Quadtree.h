#pragma once

#include <Renderer/Quadtree/QuadtreeNode.h>
#include <Renderer/Quadtree/QuadtreeOccupant.h>
#include <Renderer/Culler/Frustum.h>

#include <unordered_set>

// Base class for dynamic and static Quadtree types
class Quadtree
{
protected:
	std::unordered_set<QuadtreeOccupant*> m_outsideRoot;

	std::unique_ptr<QuadtreeNode> m_pRootNode;

	// Called whenever something is removed, an action can be defined by derived classes
	// Defaults to doing nothing
	virtual void OnRemoval();

	void SetQuadtree(QuadtreeOccupant* pOc);

public:
	virtual ~Quadtree() {}

	virtual void Add(QuadtreeOccupant* pOc) = 0;
	
	void Query_Region(const AABB2D &region, std::vector<QuadtreeOccupant*> &result) const;

	friend class QuadtreeNode;
	friend class QuadtreeOccupant;
};

