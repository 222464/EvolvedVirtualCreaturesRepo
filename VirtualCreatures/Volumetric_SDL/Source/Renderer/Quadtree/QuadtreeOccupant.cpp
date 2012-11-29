#include <Renderer/Quadtree/QuadtreeOccupant.h>

#include <Renderer/Quadtree/QuadtreeNode.h>
#include <Renderer/Quadtree/Quadtree.h>

#include <assert.h>

QuadtreeOccupant::QuadtreeOccupant()
	: m_aabb2D(Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f)),
	m_pQuadtreeNode(NULL), m_pQuadtree(NULL)
{
}

void QuadtreeOccupant::TreeUpdate()
{
	assert(m_pQuadtree != NULL);

	if(m_pQuadtreeNode == NULL)
	{
		// Not in a node, should be outside root then

		// If fits in the root now, add it
		QuadtreeNode* pRootNode = m_pQuadtree->m_pRootNode.get();

		if(pRootNode->m_region.Contains(m_aabb2D))
		{
			// Remove from outside root and add to tree
			m_pQuadtree->m_outsideRoot.erase(this);

			pRootNode->Add(this);
		}
	}
	else
		m_pQuadtreeNode->Update(this);
}

void QuadtreeOccupant::RemoveFromTree()
{
	assert(m_pQuadtree != NULL);

	if(m_pQuadtreeNode == NULL)
	{
		// Not in a node, should be outside root then
		m_pQuadtree->m_outsideRoot.erase(this);

		m_pQuadtree->OnRemoval();
	}
	else
		m_pQuadtreeNode->Remove(this);
}

const AABB2D &QuadtreeOccupant::GetAABB2D()
{
	return m_aabb2D;
}

Quadtree* QuadtreeOccupant::GetTree()
{
	return m_pQuadtree;
}