#include <Renderer/Quadtree/StaticQuadtree.h>

#include <assert.h>

StaticQuadtree::StaticQuadtree()
	: m_created(false)
{
}

StaticQuadtree::StaticQuadtree(const AABB2D &rootRegion)
	: m_created(false)
{
	m_pRootNode.reset(new QuadtreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

void StaticQuadtree::Create(const AABB2D &rootRegion)
{
	assert(!m_created);

	m_pRootNode.reset(new QuadtreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

void StaticQuadtree::Add(QuadtreeOccupant* pOc)
{
	assert(m_created);

	SetQuadtree(pOc);

	// If the occupant fits in the root node
	if(m_pRootNode->GetRegion().Contains(pOc->GetAABB2D()))
		m_pRootNode->Add(pOc);
	else
		m_outsideRoot.insert(pOc);
}

void StaticQuadtree::Clear()
{
	assert(m_created);

	m_pRootNode.reset();

	m_created = false;
}

void StaticQuadtree::Clear(const AABB2D &rootRegion)
{
	m_pRootNode.reset(new QuadtreeNode(rootRegion, 0, NULL, this));

	m_created = true;
}

AABB2D StaticQuadtree::GetRootAABB2D()
{
	assert(m_created);

	return m_pRootNode->GetRegion();
}