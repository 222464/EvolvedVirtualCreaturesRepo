#include <Renderer/Octree/OctreeOccupant.h>

struct Occlusion_Renderer
{
	virtual ~Occlusion_Renderer() {}
	virtual void DrawAABB(const AABB &aabb) = 0;
	virtual void Draw(OctreeOccupant* pOc) = 0; // Call to draw occupant
	virtual void FinishDraw() = 0; // Called when draw must have taken place
};