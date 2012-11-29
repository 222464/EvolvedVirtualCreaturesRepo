#pragma once

#include <Renderer/Octree/OctreeNode.h>
#include <Renderer/Octree/OctreeOccupant.h>
#include <Renderer/Culler/Frustum.h>

#include <unordered_set>
#include <list>

// Callback function for ray cast
typedef bool (*Octree_RayCallBack)(class OctreeOccupant* pOc, const float t[2]);

// Base class for dynamic and static octree types
class Octree
{
protected:
	std::unordered_set<OctreeOccupant*> m_outsideRoot;

	std::unique_ptr<OctreeNode> m_pRootNode;

	// Called whenever something is removed, an action can be defined by derived classes
	// Defaults to doing nothing
	virtual void OnRemoval();

	void SetOctree(OctreeOccupant* pOc);

public:
	virtual ~Octree() {}

	virtual void Add(OctreeOccupant* pOc) = 0;
	
	void Query_Region(const AABB &region, std::vector<OctreeOccupant*> &result) const;
	void Query_Frustum(std::vector<OctreeOccupant*> &result, const Frustum &frustum) const;
	void Query_Segment(std::vector<OctreeOccupant*> &result, const Vec3f &p1, const Vec3f &p2) const;

	// Get closest
	void Query_Ray(OctreeOccupant* &result, float &t, const Vec3f &start, const Vec3f &direction) const;
	void Query_Ray(OctreeOccupant* &result, float &t, const Vec3f &start, const Vec3f &direction, Octree_RayCallBack callBack) const;

	void DebugRender() const;

	friend class OctreeNode;
	friend class OctreeOccupant;
	friend class Octree_CHC;
	friend class Octree_CHCPP;
	friend class SoftwareOcclusionCuller;
};