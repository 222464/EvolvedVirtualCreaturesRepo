#pragma once

#include <System/Uncopyable.h>
#include <Constructs/AABB.h>
#include <Constructs/Point3i.h>
#include <Renderer/Octree/OctreeOccupant.h>

#include <unordered_set>

class Octree;

class OctreeNode
{
private:
	OctreeNode* m_pParent;
	Octree* m_pOctree;

	// Cannot use a unique_ptr, since the vector requires copy ctor/assignment ops
	std::vector<std::vector<std::vector<OctreeNode>>>* m_children;
	bool m_hasChildren;

	std::unordered_set<OctreeOccupant*> m_pOccupants;

	AABB m_region;

	int m_level;

	int m_numOccupantsBelow;

	inline OctreeNode* GetChild(const Point3i position);

	void GetPossibleOccupantPosition(OctreeOccupant* pOc, Point3i &point);

	void AddToThisLevel(OctreeOccupant* pOc);

	// Returns true if occupant was added to children
	bool AddToChildren(OctreeOccupant* pOc);

	void GetOccupants(std::unordered_set<OctreeOccupant*> &occupants);

	void Partition();
	void DestroyChildren();
	void Merge();

	void Update(OctreeOccupant* pOc);
	void Remove(OctreeOccupant* pOc);

public:
	static int minNumOccupants;
	static int maxNumOccupants;
	static int maxNumLevels;

	static float m_oversizeMultiplier;

	bool m_isVisible;
	int m_lastVisitedFrameID;

	OctreeNode();
	OctreeNode(const AABB &region, int level, OctreeNode* pParent, Octree* pOctree);
	~OctreeNode();

	// For use after using default constructor
	void Create(const AABB &region, int level, OctreeNode* pParent, Octree* pOctree);

	Octree* GetTree();

	void Add(OctreeOccupant* pOc);

	const AABB &GetRegion();

	void GetAllOccupantsBelow(std::vector<OctreeOccupant*> &occupants);

	int GetNumOccupantsBelow();

	friend class OctreeOccupant;
	friend class Octree;
	friend class Octree_CHC;
	friend class Octree_CHCPP;
	friend class SoftwareOcclusionCuller;
};

