#include <Renderer/Octree/Octree.h>
#include <Renderer/Octree/Occlusion_Renderer.h>

class Octree_CHC
{
private:
	struct CHC_Query
	{
		unsigned int m_query;
		OctreeNode* m_pNode;

		bool m_intersectsCamera;

		CHC_Query();
	};

	struct NodeAndDistance
	{
		OctreeNode* m_pNode;
		float m_distance;

		NodeAndDistance();
		NodeAndDistance(OctreeNode* pNode, float distance);

		static bool DistCompare(const NodeAndDistance &first, const NodeAndDistance &second);
	};

	Octree* m_pOctree;
	Occlusion_Renderer* m_pOcclusionRenderer;
	Vec3f m_cameraPosition;
	Frustum m_frustum;

	std::list<OctreeNode*> m_distanceQueue;
	std::list<CHC_Query> m_queryQueue;

	std::unordered_set<OctreeOccupant*> m_alreadyRendered;

	bool m_created;

	void HandleReturnedQuery(const CHC_Query &query);
	void TraverseNode(OctreeNode* pNode);
	void PullUpVisibility(OctreeNode* pNode);

	void IssueQuery(OctreeNode* pNode);

	bool CheckFirstQueryFinished();

	void AddDepthSortedChildrenToDistanceQueue(OctreeNode* pNode);

	unsigned int m_numOccluded;

	int m_frameID;
	int m_previousFrameID;

public:
	// Configurable parameters
	int m_wrapFrame;

	Octree_CHC();
	~Octree_CHC();

	void Create(Octree* pOctree, Occlusion_Renderer* pOcclusionRenderer);

	void Run(const Vec3f &cameraPosition, const Frustum &frustum);

	bool Created() const
	{
		return m_created;
	}
};