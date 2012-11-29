#include <Renderer/Octree/StaticOctree.h>

#include <assert.h>

StaticOctree::StaticOctree()
	: m_created(false)
{
}

StaticOctree::StaticOctree(const AABB &rootRegion)
	: m_created(false)
{
	m_pRootNode.reset(new OctreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

void StaticOctree::Create(const AABB &rootRegion)
{
	assert(!m_created);

	m_pRootNode.reset(new OctreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

void StaticOctree::Add(OctreeOccupant* pOc)
{
	assert(m_created);

	SetOctree(pOc);

	// If the occupant fits in the root node
	if(m_pRootNode->GetRegion().Contains(pOc->GetAABB()))
		m_pRootNode->Add(pOc);
	else
		m_outsideRoot.insert(pOc);
}

void StaticOctree::Clear()
{
	m_pRootNode.reset();

	m_created = false;
}

void StaticOctree::Clear(const AABB &rootRegion)
{
	m_pRootNode.reset(new OctreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

AABB StaticOctree::GetRootAABB()
{
	assert(m_created);

	return m_pRootNode->GetRegion();
}