#include <Renderer/Octree/SoftwareOcclusionCuller.h>

#include <assert.h>

bool Occluder::RayCast(const Vec3f &start, const Vec3f &end)
{
	return true;
}

SoftwareOcclusionCuller::SoftwareOcclusionCuller()
	: m_created(false)
{
}

SoftwareOcclusionCuller::~SoftwareOcclusionCuller()
{
}

void SoftwareOcclusionCuller::Create(const AABB &sptRegion)
{
	m_spt.Create(sptRegion);

	m_created = true;
}

void SoftwareOcclusionCuller::AddOccluder(Occluder* pOccluder)
{
	m_spt.Add(pOccluder);
}

void SoftwareOcclusionCuller::Query_Frustum_Occlusion(Octree &sceneSPT, const Vec3f &viewerPosition, const Frustum &frustum, std::vector<OctreeOccupant*> &result)
{
	assert(m_created);

	m_viewerPosition = viewerPosition;

	// Query outside root elements - add them if they are visible
	for(std::unordered_set<OctreeOccupant*>::iterator it = sceneSPT.m_outsideRoot.begin(); it != sceneSPT.m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(!frustum.Test_AABB_Outside(pOc->GetAABB()))
		{
			if(!RayCastOccupant(pOc))
				// Visible, add to list
				result.push_back(pOc);
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(sceneSPT.m_pRootNode.get());

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

				if(!frustum.Test_AABB_Outside(pOc->GetAABB()))
				{
					if(!RayCastOccupant(pOc))
						// Visible, add to list
						result.push_back(pOc);
				}
			}

			// Add children to open list if they are visible
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

bool SoftwareOcclusionCuller::RayCastFromViewer_IgnoreOccupant(OctreeOccupant* pOc, const Vec3f &position)
{
	OctreeOccupant* result;

	float t;
	Vec3f dir((position - m_viewerPosition).Normalize());

	m_spt.Query_Ray(result, t, m_viewerPosition, dir);

	if(result == NULL)
		return false;

	if(static_cast<Occluder*>(result)->RayCast(m_viewerPosition + dir * t, position))
		return true;
	
	return true;
}

bool SoftwareOcclusionCuller::RayCastOccupant(OctreeOccupant* pOc)
{
	// For all corners
	const Vec3f &lowerBound = pOc->GetAABB().m_lowerBound;
	const Vec3f &upperBound = pOc->GetAABB().m_upperBound;

	// Front side
	if(RayCastFromViewer_IgnoreOccupant(pOc, lowerBound))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(upperBound.x, lowerBound.y, lowerBound.z)))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(lowerBound.x, upperBound.y, lowerBound.z)))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(upperBound.x, upperBound.y, lowerBound.z)))
		return true;
	// Back side
	else if(RayCastFromViewer_IgnoreOccupant(pOc, upperBound))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(upperBound.x, lowerBound.y, upperBound.z)))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(lowerBound.x, upperBound.y, upperBound.z)))
		return true;
	else if(RayCastFromViewer_IgnoreOccupant(pOc, Vec3f(upperBound.x, upperBound.y, upperBound.z)))
		return true;

	return false;
}