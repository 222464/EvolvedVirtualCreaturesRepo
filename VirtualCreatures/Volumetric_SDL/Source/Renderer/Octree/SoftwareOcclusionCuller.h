#pragma once

#include <Renderer/Octree/StaticOctree.h>
#include <Renderer/Octree/Occlusion_Renderer.h>

struct Occluder :
	public OctreeOccupant
{
	virtual bool RayCast(const Vec3f &start, const Vec3f &end);
};

class SoftwareOcclusionCuller
{
private:
	StaticOctree m_spt;

	bool RayCastFromViewer_IgnoreOccupant(OctreeOccupant* pOc, const Vec3f &position);
	bool RayCastOccupant(OctreeOccupant* pOc);

	Vec3f m_viewerPosition;

	bool m_created;

public:
	SoftwareOcclusionCuller();
	~SoftwareOcclusionCuller();

	void Create(const AABB &sptRegion);

	// Does NOT delete for you. Only holds reference.
	void AddOccluder(Occluder* pOccluder); 

	void Query_Frustum_Occlusion(Octree &sceneSPT, const Vec3f &viewerPosition, const Frustum &frustum, std::vector<OctreeOccupant*> &result);
};

