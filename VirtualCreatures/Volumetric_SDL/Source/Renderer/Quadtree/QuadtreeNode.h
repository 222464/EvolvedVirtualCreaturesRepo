#pragma once

#include <System/Uncopyable.h>
#include <Constructs/AABB2D.h>
#include <Constructs/Point2i.h>
#include <Renderer/Quadtree/QuadtreeOccupant.h>

#include <unordered_set>

class Quadtree;

class QuadtreeNode
{
private:
	QuadtreeNode* m_pParent;
	Quadtree* m_pQuadtree;

	// Cannot use a unique_ptr, since the vector requires copy ctor/assignment ops
	std::vector<std::vector<QuadtreeNode>>* m_children;
	bool m_hasChildren;

	std::unordered_set<QuadtreeOccupant*> m_pOccupants;

	AABB2D m_region;

	int m_level;

	int m_numOccupantsBelow;

	inline QuadtreeNode* GetChild(const Point2i position);

	void GetPossibleOccupantPosition(QuadtreeOccupant* pOc, Point2i &point);

	void AddToThisLevel(QuadtreeOccupant* pOc);

	// Returns true if occupant was added to children
	bool AddToChildren(QuadtreeOccupant* pOc);

	void GetOccupants(std::unordered_set<QuadtreeOccupant*> &occupants);

	void Partition();
	void DestroyChildren();
	void Merge();

	void Update(QuadtreeOccupant* pOc);
	void Remove(QuadtreeOccupant* pOc);

public:
	static int minNumOccupants;
	static int maxNumOccupants;
	static int maxNumLevels;

	static float m_oversizeMultiplier;

	QuadtreeNode();
	QuadtreeNode(const AABB2D &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree);
	~QuadtreeNode();

	// For use after using default constructor
	void Create(const AABB2D &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree);

	Quadtree* GetTree();

	void Add(QuadtreeOccupant* pOc);

	const AABB2D &GetRegion();

	void GetAllOccupantsBelow(std::vector<QuadtreeOccupant*> &occupants);

	int GetNumOccupantsBelow();

	friend class QuadtreeOccupant;
	friend class Quadtree;
};

