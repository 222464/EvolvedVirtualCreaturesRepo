#include <Renderer/Octree/Octree.h>
#include <Renderer/Octree/Occlusion_Renderer.h>

class Octree_CHCPP
{
private:
	struct CHCPP_Query
	{
		unsigned int m_query;
		std::vector<OctreeNode*> m_pNodes; // Vector so that can use with multiqueries

		bool m_intersectsCamera;

		void SetAllNodesVisibility(bool visible) const;

		CHCPP_Query();
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
	std::list<CHCPP_Query> m_queryQueue;
	std::list<OctreeNode*> m_visibleQueue;
	std::list<OctreeNode*> m_invisibleQueue;

	std::unordered_set<OctreeOccupant*> m_alreadyRendered;

	bool m_created;

	void HandleReturnedQuery(const CHCPP_Query &query);
	void TraverseNode(OctreeNode* pNode);
	void PullUpVisibility(OctreeNode* pNode);
	void QueryPreviouslyInvisibleNode(OctreeNode* pNode);
	void IssueMultiQueries();

	void QueryIndividualNodes(const std::vector<OctreeNode*> &nodes);
	void IssueQuery(OctreeNode* pNode);

	bool CheckForFinishedQuery(CHCPP_Query &query);

	void AddDepthSortedChildrenToDistanceQueue(OctreeNode* pNode);

	bool QueryReasonable(OctreeNode* pNode);

	bool WasVisible(OctreeNode* pNode);

	unsigned int m_numOccluded;

	int m_frameID;
	int m_previousFrameID;

public:
	// Configurable parameters
	unsigned int m_maxPreviouslyInvisibleNodeBatchSize;
	int m_minQueryNodeOccupantCount;
	unsigned int m_maxMultiQueryNodeCount;
	int m_wrapFrame;

	Octree_CHCPP();
	~Octree_CHCPP();

	void Create(Octree* pOctree, Occlusion_Renderer* pCHCPPRenderer);

	void Run(const Vec3f &cameraPosition, const Frustum &frustum);

	bool Created() const
	{
		return m_created;
	}
};