#include <Renderer/Octree/Octree.h>

#include <Renderer/Octree/OctreeNode.h>
#include <Renderer/Octree/Octree.h>

#include <Renderer/Culler/Frustum.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

void Octree::OnRemoval()
{
}

void Octree::SetOctree(OctreeOccupant* pOc)
{
	pOc->m_pOctree = this;
}

void Octree::Query_Region(const AABB &region, std::vector<OctreeOccupant*> &result) const
{
	// Query outside root elements
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(region.Intersects(pOc->m_aabb))
		{
			// Intersects, add to list
			result.push_back(pOc);
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if(region.Intersects(pCurrent->m_region))
		{
			for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				OctreeOccupant* pOc = *it;

				if(region.Intersects(pOc->m_aabb))
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
						for(int z = 0; z < 2; z++)
						{
							if((*pCurrent->m_children)[x][y][z].GetNumOccupantsBelow() != 0)
								open.push_back(&(*pCurrent->m_children)[x][y][z]);
						}
			}
		}
	}
}

void Octree::Query_Frustum(std::vector<OctreeOccupant*> &result, const Frustum &frustum) const
{
	// Query outside root elements - add them if they are visible
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(!frustum.Test_AABB_Outside(pOc->m_aabb))
		{
			// Visible, add to list
			result.push_back(pOc);
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		switch(frustum.Test_AABB(pCurrent->m_region))
		{
		case Frustum::inside:
			// Add all of this nodes occupants and those below, since all must be visibile
			pCurrent->GetAllOccupantsBelow(result);
			
			break;

		case Frustum::intersect:
			// Add occupants if they are visible
			for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				OctreeOccupant* pOc = *it;

				if(!frustum.Test_AABB_Outside(pOc->m_aabb))
				{
					// Visible, add to list
					result.push_back(pOc);
				}
			}

			// Add children to open list
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
						for(int z = 0; z < 2; z++)
						{
							if((*pCurrent->m_children)[x][y][z].GetNumOccupantsBelow() != 0)
								open.push_back(&(*pCurrent->m_children)[x][y][z]);
						}
			}

			break;

			// Outside case is ignored
		}
	}
}


void Octree::Query_Segment(std::vector<OctreeOccupant*> &result, const Vec3f &p1, const Vec3f &p2) const
{
	// Query outside root elements
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(pOc->m_aabb.Intersects(p1, p2))
		{
			// Intersects, add to list
			result.push_back(pOc);
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if(pCurrent->m_region.Intersects(p1, p2))
		{
			for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				OctreeOccupant* pOc = *it;

				if(pOc->m_aabb.Intersects(p1, p2))
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
						for(int z = 0; z < 2; z++)
						{
							if((*pCurrent->m_children)[x][y][z].GetNumOccupantsBelow() != 0)
								open.push_back(&(*pCurrent->m_children)[x][y][z]);
						}
			}
		}
	}
}

void Octree::Query_Ray(OctreeOccupant* &result, float &t, const Vec3f &start, const Vec3f &direction) const
{
	result = NULL;

	t = 999999.0f;

	float ts[2];

	// Query outside root elements
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(pOc->m_aabb.Intersects(start, direction, ts))
		{
			float dist = std::min(ts[0], ts[1]);

			if(dist < t)
			{
				result = pOc;

				t = dist;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if(pCurrent->m_region.Intersects(start, direction, ts))
		{
			for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				OctreeOccupant* pOc = *it;

				if(pOc->m_aabb.Intersects(start, direction, ts))
				{
					float dist = std::min(ts[0], ts[1]);

					if(dist < t)
					{
						result = pOc;

						t = dist;
					}
				}
			}

			// Add children to open list if they intersect the region
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
						for(int z = 0; z < 2; z++)
						{
							if((*pCurrent->m_children)[x][y][z].GetNumOccupantsBelow() != 0)
								open.push_back(&(*pCurrent->m_children)[x][y][z]);
						}
			}
		}
	}
}

void Octree::Query_Ray(OctreeOccupant* &result, float &t, const Vec3f &start, const Vec3f &direction, Octree_RayCallBack callBack) const
{
	result = NULL;

	t = 999999.0f;

	float ts[2];

	// Query outside root elements
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(pOc->m_aabb.Intersects(start, direction, ts))
		{
			float dist = std::min(ts[0], ts[1]);

			if(dist < t && callBack(pOc, ts))
			{
				result = pOc;

				t = dist;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if(pCurrent->m_region.Intersects(start, direction, ts))
		{
			for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				OctreeOccupant* pOc = *it;

				if(pOc->m_aabb.Intersects(start, direction, ts))
				{
					float dist = std::min(ts[0], ts[1]);

					if(dist < t && callBack(pOc, ts))
					{
						result = pOc;

						t = dist;
					}
				}
			}

			// Add children to open list if they intersect the region
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
						for(int z = 0; z < 2; z++)
						{
							if((*pCurrent->m_children)[x][y][z].GetNumOccupantsBelow() != 0)
								open.push_back(&(*pCurrent->m_children)[x][y][z]);
						}
			}
		}
	}
}

void Octree::DebugRender() const
{
	// Render outside root AABB's
	glColor3f(0.5f, 0.2f, 0.1f);

	for(std::unordered_set<OctreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
		(*it)->m_aabb.DebugRender();

	// Now draw the tree
	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		// Render node region AABB
		glColor3f(0.4f, 0.9f, 0.7f);

		pCurrent->m_region.DebugRender();

		glColor3f(0.9f, 0.2f, 0.5f);

		// Render occupants
		for(std::unordered_set<OctreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
		{
			OctreeOccupant* pOc = *it;

			//if(AABBInFrustum(pOc->m_aabb))
			//{
				// Visible, add to list
			pOc->m_aabb.DebugRender();
			//}
		}

		// Add children to open list if they are visible
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					for(int z = 0; z < 2; z++)
					{
						//if(AABBInFrustum((*pCurrent->m_children)[x][y][z].m_region))
						open.push_back(&(*pCurrent->m_children)[x][y][z]);
							
					}
		}
	}
}

/*void Octree::UpdateVisibility()
{
	std::list<OctreeNode*> open;

	open.push_back(m_pRootNode.get());

	while(!open.empty())
	{
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		pCurrent->UpdateVisibility();

		// Add children to open list if they intersect the region
		if(pCurrent->m_hasChildren)
		{
			for(int x = 0; x < 2; x++)
				for(int y = 0; y < 2; y++)
					for(int z = 0; z < 2; z++)
						open.push_back(&(*pCurrent->m_children)[x][y][z]);
		}
	}
}*/