#include <Renderer/Quadtree/Quadtree.h>

#include <Renderer/Quadtree/QuadtreeNode.h>
#include <Renderer/Quadtree/Quadtree.h>

#include <Renderer/Culler/Frustum.h>

#include <Renderer/SDL_OpenGL.h>

void Quadtree::OnRemoval()
{
}

void Quadtree::SetQuadtree(QuadtreeOccupant* pOc)
{
	pOc->m_pQuadtree = this;
}

void Quadtree::Query_Region(const AABB2D &region, std::vector<QuadtreeOccupant*> &result) const
{
	// Query outside root elements
	for(std::unordered_set<QuadtreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		QuadtreeOccupant* pOc = *it;

		if(region.Intersects(pOc->m_aabb2D))
		{
			// Intersects, add to list
			result.push_back(pOc);
		}
	}

	std::list<QuadtreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		QuadtreeNode* pCurrent = open.back();
		open.pop_back();

		if(region.Intersects(pCurrent->m_region))
		{
			for(std::unordered_set<QuadtreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				QuadtreeOccupant* pOc = *it;

				if(region.Intersects(pOc->m_aabb2D))
				{
					// Visible, add to list
					result.push_back(pOc);
				}
			}

			// Add children to open list if they intersect the region
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
					{
						if((*pCurrent->m_children)[x][y].GetNumOccupantsBelow() != 0)
							open.push_back(&(*pCurrent->m_children)[x][y]);
					}
			}
		}
	}
}