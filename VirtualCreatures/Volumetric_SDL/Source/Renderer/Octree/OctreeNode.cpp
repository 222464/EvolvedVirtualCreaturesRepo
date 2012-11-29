#include <Renderer/Octree/OctreeNode.h>

#include <Renderer/Octree/Octree.h>

#include <assert.h>

// Defaults
int OctreeNode::minNumOccupants = 2;
int OctreeNode::maxNumOccupants = 6;
int OctreeNode::maxNumLevels = 40;
float OctreeNode::m_oversizeMultiplier = 1.3f;

OctreeNode::OctreeNode()
	: m_hasChildren(false), m_numOccupantsBelow(0),
	m_isVisible(true), m_lastVisitedFrameID(0)
{
}

OctreeNode::OctreeNode(const AABB &region, int level, OctreeNode* pParent, Octree* pOctree)
	: m_region(region), m_level(level), m_pParent(pParent), m_pOctree(pOctree),
	m_hasChildren(false), m_numOccupantsBelow(0),
	m_isVisible(true), m_lastVisitedFrameID(0)
{
}

OctreeNode::~OctreeNode()
{
	if(m_hasChildren)
		DestroyChildren();
}

void OctreeNode::Create(const AABB &region, int level, OctreeNode* pParent, Octree* pOctree)
{
	m_region = region;
	m_level = level;
	m_pParent = pParent;
	m_pOctree = pOctree;
}

inline OctreeNode* OctreeNode::GetChild(const Point3i position)
{
	return &(*m_children)[position.x][position.y][position.z];
}

void OctreeNode::GetPossibleOccupantPosition(OctreeOccupant* pOc, Point3i &point)
{
	// Compare the center of the AABB of the occupant to that of this node to determine
	// which child it may (possibly, not certainly) fit in
	const Vec3f &occupantCenter(pOc->m_aabb.GetCenter());
	const Vec3f &nodeRegionCenter(m_region.GetCenter());

	if(occupantCenter.x > nodeRegionCenter.x)
		point.x = 1;
	else
		point.x = 0;

	if(occupantCenter.y > nodeRegionCenter.y)
		point.y = 1;
	else
		point.y = 0;

	if(occupantCenter.z > nodeRegionCenter.z)
		point.z = 1;
	else
		point.z = 0;
}

void OctreeNode::AddToThisLevel(OctreeOccupant* pOc)
{
	pOc->m_pOctreeNode = this;

	m_pOccupants.insert(pOc);
}

bool OctreeNode::AddToChildren(OctreeOccupant* pOc)
{
	assert(m_hasChildren);

	Point3i position;

	GetPossibleOccupantPosition(pOc, position);

	OctreeNode* pChild = GetChild(position);

	// See if the occupant fits in the child at the selected position
	if(pChild->m_region.Contains(pOc->m_aabb))
	{
		// Fits, so can add to the child and finish
		pChild->Add(pOc);

		return true;
	}

	return false;
}

void OctreeNode::Partition()
{
	assert(!m_hasChildren);

	const Vec3f &halfRegionDims(m_region.GetHalfDims());
	const Vec3f &regionLowerBound(m_region.GetLowerBound());
	const Vec3f &regionCenter(m_region.GetCenter());

	int nextLowerLevel = m_level - 1;

	m_children = new std::vector<std::vector<std::vector<OctreeNode>>>();

	// Create the children nodes
	(*m_children).resize(2);

	for(int x = 0; x < 2; x++)
	{
		(*m_children)[x].resize(2);

		for(int y = 0; y < 2; y++)
		{
			(*m_children)[x][y].resize(2);

			for(int z = 0; z < 2; z++)
			{
				Vec3f offset(x * halfRegionDims.x, y * halfRegionDims.y, z * halfRegionDims.z);

				AABB childAABB(regionLowerBound + offset, regionCenter + offset);

				childAABB.SetHalfDims(childAABB.GetHalfDims() * m_oversizeMultiplier);

				// Scale up AABB by the oversize multiplier

				(*m_children)[x][y][z].Create(childAABB, nextLowerLevel, this, m_pOctree);
			}
		}
	}

	m_hasChildren = true;
}

void OctreeNode::DestroyChildren()
{
	assert(m_hasChildren);

	delete m_children;

	m_hasChildren = false;
}

void OctreeNode::Merge()
{
	if(m_hasChildren)
	{
		// Place all occupants at lower levels into this node
		GetOccupants(m_pOccupants);

		DestroyChildren();
	}
}

void OctreeNode::GetOccupants(std::unordered_set<OctreeOccupant*> &occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<OctreeNode*> open;

	open.push_back(this);

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
		{
			// Assign new node
			(*it)->m_pOctreeNode = this;

			// Add to this node
			occupants.insert(*it);
		}

		// If the node has children, add them to the open list
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					for(int z = 0; z < 2; z++)
						open.push_back(&(*pCurrent->m_children)[x][y][z]);
		}
	}
}

void OctreeNode::GetAllOccupantsBelow(std::vector<OctreeOccupant*> &occupants)
{
	// Iteratively parse subnodes in order to collect all occupants below this node
	std::list<OctreeNode*> open;

	open.push_back(this);

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		// Get occupants
		for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			// Add to this node
			occupants.push_back(*it);

		// If the node has children, add them to the open list
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					for(int z = 0; z < 2; z++)
						open.push_back(&(*pCurrent->m_children)[x][y][z]);
		}
	}
}

void OctreeNode::Update(OctreeOccupant* pOc)
{
	// Remove, may be re-added to this node later
	m_pOccupants.erase(pOc);

	// Propogate upwards, looking for a node that has room (the current one may still have room)
	OctreeNode* pNode = this;

	while(pNode != NULL)
	{
		pNode->m_numOccupantsBelow--;

		// If has room for 1 more, found a spot
		if(pNode->m_region.Contains(pOc->m_aabb))
			break;

		pNode = pNode->m_pParent;
	}

	// If no node that could contain the occupant was found, add to outside root set
	if(pNode == NULL)
	{
		assert(m_pOctree != NULL);

		m_pOctree->m_outsideRoot.insert(pOc);

		pOc->m_pOctreeNode = NULL;
	}
	else // Add to the selected node
		pNode->Add(pOc);
}

void OctreeNode::Remove(OctreeOccupant* pOc)
{
	assert(!m_pOccupants.empty());

	// Remove from node
	m_pOccupants.erase(pOc);

	// Propogate upwards, merging if there are enough occupants in the node
	OctreeNode* pNode = this;

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

void OctreeNode::Add(OctreeOccupant* pOc)
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

Octree* OctreeNode::GetTree()
{
	return m_pOctree;
}

const AABB &OctreeNode::GetRegion()
{
	return m_region;
}

int OctreeNode::GetNumOccupantsBelow()
{
	return m_numOccupantsBelow;
}