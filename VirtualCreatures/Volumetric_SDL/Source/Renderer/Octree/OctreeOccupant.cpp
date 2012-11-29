#include <Renderer/Octree/OctreeOccupant.h>

#include <Renderer/Octree/OctreeNode.h>
#include <Renderer/Octree/Octree.h>

#include <assert.h>

OctreeOccupant::OctreeOccupant()
	: m_aabb(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f)),
	m_pOctreeNode(NULL), m_pOctree(NULL)
{
}

void OctreeOccupant::TreeUpdate()
{
	assert(m_pOctree != NULL);

	if(m_pOctreeNode == NULL)
	{
		// Not in a node, should be outside root then

		// If fits in the root now, add it
		OctreeNode* pRootNode = m_pOctree->m_pRootNode.get();

		if(pRootNode->m_region.Contains(m_aabb))
		{
			// Remove from outside root and add to tree
			m_pOctree->m_outsideRoot.erase(this);

			pRootNode->Add(this);
		}
	}
	else
		m_pOctreeNode->Update(this);
}

void OctreeOccupant::RemoveFromTree()
{
	assert(m_pOctree != NULL);

	if(m_pOctreeNode == NULL)
	{
		// Not in a node, should be outside root then
		m_pOctree->m_outsideRoot.erase(this);

		m_pOctree->OnRemoval();
	}
	else
		m_pOctreeNode->Remove(this);
}

const AABB &OctreeOccupant::GetAABB()
{
	return m_aabb;
}

Octree* OctreeOccupant::GetTree()
{
	return m_pOctree;
}