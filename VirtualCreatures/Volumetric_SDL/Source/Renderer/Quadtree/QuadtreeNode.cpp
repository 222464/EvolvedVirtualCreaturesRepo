#include <Renderer/Quadtree/QuadtreeNode.h>

#include <Renderer/Quadtree/Quadtree.h>

#include <assert.h>

// Defaults
int QuadtreeNode::minNumOccupants = 1;
int QuadtreeNode::maxNumOccupants = 8;
int QuadtreeNode::maxNumLevels = 20;
float QuadtreeNode::m_oversizeMultiplier = 1.0f;

QuadtreeNode::QuadtreeNode()
	: m_hasChildren(false), m_numOccupantsBelow(0)
{
}

QuadtreeNode::QuadtreeNode(const AABB2D &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree)
	: m_region(region), m_level(level), m_pParent(pParent), m_pQuadtree(pQuadtree),
	m_hasChildren(false), m_numOccupantsBelow(0)
{
}

QuadtreeNode::~QuadtreeNode()
{
	if(m_hasChildren)
		DestroyChildren();
}

void QuadtreeNode::Create(const AABB2D &region, int level, QuadtreeNode* pParent, Quadtree* pQuadtree)
{
	m_region = region;
	m_level = level;
	m_pParent = pParent;
	m_pQuadtree = pQuadtree;
}

inline QuadtreeNode* QuadtreeNode::GetChild(const Point2i position)
{
	return &(*m_children)[position.x][position.y];
}

void QuadtreeNode::GetPossibleOccupantPosition(QuadtreeOccupant* pOc, Point2i &point)
{
	// Compare the center of the AABB2D of the occupant to that of this node to determine
	// which child it may (possibly, not certainly) fit in
	const Vec2f &occupantCenter(pOc->m_aabb2D.GetCenter());
	const Vec2f &nodeRegionCenter(m_region.GetCenter());

	if(occupantCenter.x > nodeRegionCenter.x)
		point.x = 1;
	else
		point.x = 0;

	if(occupantCenter.y > nodeRegionCenter.y)
		point.y = 1;
	else
		point.y = 0;
}

void QuadtreeNode::AddToThisLevel(QuadtreeOccupant* pOc)
{
	pOc->m_pQuadtreeNode = this;

	m_pOccupants.insert(pOc);
}

bool QuadtreeNode::AddToChildren(QuadtreeOccupant* pOc)
{
	assert(m_hasChildren);

	Point2i position;

	GetPossibleOccupantPosition(pOc, position);

	QuadtreeNode* pChild = GetChild(position);

	// See if the occupant fits in the child at the selected position
	if(pChild->m_region.Contains(pOc->m_aabb2D))
	{
		// Fits, so can add to the child and finish
		pChild->Add(pOc);

		return true;
	}

	return false;
}

void QuadtreeNode::Partition()
{
	assert(!m_hasChildren);

	const Vec2f &halfRegionDims(m_region.GetHalfDims());
	const Vec2f &regionLowerBound(m_region.GetLowerBound());
	const Vec2f &regionCenter(m_region.GetCenter());

	int nextLowerLevel = m_level - 1;

	m_children = new std::vector<std::vector<QuadtreeNode>>();

	// Create the children nodes
	(*m_children).resize(2);

	for(int x = 0; x < 2; x++)
	{
		(*m_children)[x].resize(2);

		for(int y = 0; y < 2; y++)
		{
			Vec2f offset(x * halfRegionDims.x, y * halfRegionDims.y);

			AABB2D childAABB2D(regionLowerBound + offset, regionCenter + offset);

			childAABB2D.SetHalfDims(childAABB2D.GetHalfDims() * m_oversizeMultiplier);

			// Scale up AABB2D by the oversize multiplier

			(*m_children)[x][y].Create(childAABB2D, nextLowerLevel, this, m_pQuadtree);
		}
	}

	m_hasChildren = true;
}

void QuadtreeNode::DestroyChildren()
{
	assert(m_hasChildren);

	delete m_children;

	m_hasChildren = false;
}

void QuadtreeNode::Merge()
{
	if(m_hasChildren)
	{
		// Place all occupants at lower levels into this node
		GetOccupants(m_pOccupants);

		DestroyChildren();
	}
}

void QuadtreeNode::GetOccupants(std::unordered_set<QuadtreeOccupant*> &occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;

	open.push_back(this);

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for(std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
		{
			// Assign new node
			(*it)->m_pQuadtreeNode = this;

			// Add to this node
			occupants.insert(*it);
		}

		// If the node has children, add them to the open list
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					open.push_back(&(*pCurrent->m_children)[x][y]);
		}
	}
}

void QuadtreeNode::GetAllOccupantsBelow(std::vector<QuadtreeOccupant*> &occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<QuadtreeNode*> open;

	open.push_back(this);

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for(std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			// Add to this node
			occupants.push_back(*it);

		// If the node has children, add them to the open list
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					open.push_back(&(*pCurrent->m_children)[x][y]);
		}
	}
}

void QuadtreeNode::Update(QuadtreeOccupant* pOc)
{
	// Remove, may be re-added to this node later
	m_pOccupants.erase(pOc);

	// Propogate upwards, looking for a node that has room (the current one may still have room)
	QuadtreeNode* pNode = this;

	while(pNode != NULL)
	{
		pNode->m_numOccupantsBelow--;

		// If has room for 1 more, found a spot
		if(pNode->m_region.Contains(pOc->m_aabb2D))
			break;

		pNode = pNode->m_pParent;
	}

	// If no node that could contain the occupant was found, add to outside root set
	if(pNode == NULL)
	{
		assert(m_pQuadtree != NULL);

		m_pQuadtree->m_outsideRoot.insert(pOc);

		pOc->m_pQuadtreeNode = NULL;
	}
	else // Add to the selected node
		pNode->Add(pOc);
}

void QuadtreeNode::Remove(QuadtreeOccupant* pOc)
{
	assert(!m_pOccupants.empty());

	// Remove from node
	m_pOccupants.erase(pOc);

	// Propogate upwards, merging if there are enough occupants in the node
	QuadtreeNode* pNode = this;

	while(pNode != NULL)
	{
		pNode->m_numOccupantsBelow--;

		if(pNode->m_numOccupantsBelow >= minNumOccupants)
		{
			pNode->Merge();

			break;
		}

		pNode = pNode->m_pParent;
	}
}

void QuadtreeNode::Add(QuadtreeOccupant* pOc)
{
	m_numOccupantsBelow++;

	// See if the occupant fits into any children (if there are any)
	if(m_hasChildren)
	{
		if(AddToChildren(pOc))
			return; // Fit, can stop
	}
	else
	{
		// Check if we need a new partition
		if(static_cast<signed>(m_pOccupants.size()) >= maxNumOccupants && m_level < maxNumLevels)
		{
			Partition();

			if(AddToChildren(pOc))
				return;
		}
	}

	// Did not fit in anywhere, add to this level, even if it goes over the maximum size
	AddToThisLevel(pOc);
}

Quadtree* QuadtreeNode::GetTree()
{
	return m_pQuadtree;
}

const AABB2D &QuadtreeNode::GetRegion()
{
	return m_region;
}

int QuadtreeNode::GetNumOccupantsBelow()
{
	return m_numOccupantsBelow;
}