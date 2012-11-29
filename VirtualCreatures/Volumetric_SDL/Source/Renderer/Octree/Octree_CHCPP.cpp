#include <Renderer/Octree/Octree_CHCPP.h>

#include <Renderer/Octree/OctreeNode.h>
#include <Renderer/Octree/Octree.h>

#include <Renderer/Culler/Frustum.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

#include <algorithm>

Octree_CHCPP::CHCPP_Query::CHCPP_Query()
	: m_query(0), m_intersectsCamera(false)
{
}

void Octree_CHCPP::CHCPP_Query::SetAllNodesVisibility(bool visible) const
{
	for(unsigned int i = 0, size = m_pNodes.size(); i < size; i++)
		m_pNodes[i]->m_isVisible = visible;
}

Octree_CHCPP::NodeAndDistance::NodeAndDistance()
{
}

Octree_CHCPP::NodeAndDistance::NodeAndDistance(OctreeNode* pNode, float distance)
	: m_pNode(pNode), m_distance(distance)
{
}

bool Octree_CHCPP::NodeAndDistance::DistCompare(const NodeAndDistance &first, const NodeAndDistance &second)
{
	return first.m_distance < second.m_distance;
}

Octree_CHCPP::Octree_CHCPP()
	: m_created(false),
	m_maxPreviouslyInvisibleNodeBatchSize(4),
	m_minQueryNodeOccupantCount(26),
	m_maxMultiQueryNodeCount(4),
	m_frameID(0), m_wrapFrame(4000)
{
}

Octree_CHCPP::~Octree_CHCPP()
{
	// Delete left over queries
	for(std::list<CHCPP_Query>::iterator it = m_queryQueue.begin(); it != m_queryQueue.end(); it++)
		glDeleteQueries(1, &(*it).m_query);
}

void Octree_CHCPP::Create(Octree* pOctree, Occlusion_Renderer* pCHCPPRenderer)
{
	m_pOctree = pOctree;
	m_pOcclusionRenderer = pCHCPPRenderer;

	m_created = true;
}

void Octree_CHCPP::HandleReturnedQuery(const CHCPP_Query &query)
{
	assert(!query.m_pNodes.empty());

	if(query.m_intersectsCamera)
	{
		// If was a multiquery (a failed one, since it is visible)
		if(query.m_pNodes.size() > 1)
			QueryIndividualNodes(query.m_pNodes);
		else
		{
			if(!WasVisible(query.m_pNodes[0]))
				TraverseNode(query.m_pNodes[0]);

			PullUpVisibility(query.m_pNodes[0]);
		}
	}
	else
	{
		assert(query.m_query != 0);

		// Check how many fragments passed the test. If none passed, this chunk is occluded, and must not be rendered
		GLuint anySamplesPassed;
		glGetQueryObjectuiv(query.m_query, GL_QUERY_RESULT, &anySamplesPassed);

		// Delete query
		glDeleteQueries(1, &query.m_query);

		// Visible
		if(anySamplesPassed)
		{
			// If was a multiquery (a failed one, since it is visible)
			if(query.m_pNodes.size() > 1)
				QueryIndividualNodes(query.m_pNodes);
			else
			{
				if(!WasVisible(query.m_pNodes[0]))
					TraverseNode(query.m_pNodes[0]);

				PullUpVisibility(query.m_pNodes[0]);
			}
		}
		else // Occluded, set to invisible (more than one if it was a multiquery)
		{
			query.SetAllNodesVisibility(false);

			m_numOccluded++;
		}
	}
}

void Octree_CHCPP::TraverseNode(OctreeNode* pNode)
{
	if(pNode->m_hasChildren)
	{
		AddDepthSortedChildrenToDistanceQueue(pNode);
		pNode->m_isVisible = false;
	}
	
	// Draw all occupants of node
	for(std::unordered_set<OctreeOccupant*>::iterator it = pNode->m_pOccupants.begin(); it != pNode->m_pOccupants.end(); it++)
	{
		// If in the frustum
		if(!m_frustum.Test_AABB_Outside((*it)->GetAABB()))
			m_pOcclusionRenderer->Draw(*it);
	}

	// Draw all occupants of node
	//for(std::unordered_set<OctreeOccupant*>::iterator it = pNode->m_pOccupants.begin(); it != pNode->m_pOccupants.end(); it++)
	//{
	//	OctreeOccupant* pOccupant = *it;

	//	std::unordered_set<OctreeOccupant*>::iterator it_find = m_alreadyRendered.find(pOccupant);

	//	if(it_find == m_alreadyRendered.end())
	//	{
	//		// If in the frustum
	//		if(!m_frustum.Test_AABB_Outside(pOccupant->GetAABB()))
	//			m_pOcclusionRenderer->Draw(pOccupant);

	//		m_alreadyRendered.insert(pOccupant);
	//	}
	//}
}

void Octree_CHCPP::PullUpVisibility(OctreeNode* pNode)
{
	while(pNode != NULL && !pNode->m_isVisible)
	{
		pNode->m_isVisible = true;
		pNode = pNode->m_pParent;
	}
}

void Octree_CHCPP::QueryPreviouslyInvisibleNode(OctreeNode* pNode)
{
	m_invisibleQueue.push_back(pNode);

	// If large enough count start using multiqueries
	if(m_invisibleQueue.size() >= m_maxPreviouslyInvisibleNodeBatchSize)
		IssueMultiQueries();
}

void Octree_CHCPP::IssueMultiQueries()
{
	// Group all into one query if none intersect the camera. There are more efficient ways to do this (separate queries based on locality)
	// But for now all are put in 1 single query
	if(m_invisibleQueue.empty())
		return;

	// Make sure rendering is done before query is assigned
	m_pOcclusionRenderer->FinishDraw();

	// Organize into camera intersecting and not camera intersecting lists
	std::list<OctreeNode*> cameraIntersecting; // Nodes left over in m_invisibleQueue do not intersect camera

	// Add all nodes to query
	for(std::list<OctreeNode*>::iterator it = m_invisibleQueue.begin(); it != m_invisibleQueue.end();)
	{
		if((*it)->GetRegion().Contains(m_cameraPosition))
		{
			cameraIntersecting.push_back(*it);

			it = m_invisibleQueue.erase(it);
		}
		else
			it++;
	}

	// Multiquery
	if(!m_invisibleQueue.empty())
	{
		// Add all nodes to query
		for(std::list<OctreeNode*>::iterator it = m_invisibleQueue.begin(); it != m_invisibleQueue.end();)
		{
			CHCPP_Query newQuery;

			glGenQueries(1, &newQuery.m_query);

			glBeginQuery(GL_ANY_SAMPLES_PASSED, newQuery.m_query);

			for(unsigned int batchSize = 0; batchSize < m_maxMultiQueryNodeCount && it != m_invisibleQueue.end(); batchSize++, it++)
			{	
				newQuery.m_pNodes.push_back(*it);

				m_pOcclusionRenderer->DrawAABB((*it)->GetRegion());
			}

			glEndQuery(GL_ANY_SAMPLES_PASSED);

			// Add to queue
			m_queryQueue.push_back(newQuery);
		}
	}

	// One query for camera intersectors
	if(!cameraIntersecting.empty())
	{
		CHCPP_Query newQuery;

		// Add all nodes to query
		for(std::list<OctreeNode*>::iterator it = cameraIntersecting.begin(); it != cameraIntersecting.end(); it++)
			newQuery.m_pNodes.push_back(*it);

		newQuery.m_intersectsCamera = true;

		// Add to queue
		m_queryQueue.push_back(newQuery);
	}

	m_invisibleQueue.clear();
}

void Octree_CHCPP::QueryIndividualNodes(const std::vector<OctreeNode*> &nodes)
{
	for(unsigned int i = 0, size = nodes.size(); i < size; i++)
	{
		if(QueryReasonable(nodes[i]))
			IssueQuery(nodes[i]);
		else
		{
			// Render everything below node
			std::vector<OctreeOccupant*> nodeSet;

			nodes[i]->GetAllOccupantsBelow(nodeSet);

			for(unsigned int i = 0, size = nodeSet.size(); i < size; i++)
			{
				OctreeOccupant* pOc = nodeSet[i];

				if(!m_frustum.Test_AABB_Outside(pOc->GetAABB()))
					m_pOcclusionRenderer->Draw(pOc);
			}
		}
	}
}

void Octree_CHCPP::IssueQuery(OctreeNode* pNode)
{
	// If it intersects the bounding volume, no query is necessary
	if(pNode->GetRegion().Contains(m_cameraPosition))
	{
		CHCPP_Query newQuery;

		newQuery.m_pNodes.push_back(pNode);

		newQuery.m_intersectsCamera = true;

		// Add to queue
		m_queryQueue.push_back(newQuery);
	}
	else
	{
		// Make sure rendering is done before query is assigned
		m_pOcclusionRenderer->FinishDraw();

		CHCPP_Query newQuery;

		newQuery.m_pNodes.push_back(pNode);

		glGenQueries(1, &newQuery.m_query);

		glBeginQuery(GL_ANY_SAMPLES_PASSED, newQuery.m_query);

		m_pOcclusionRenderer->DrawAABB(pNode->GetRegion());

		glEndQuery(GL_ANY_SAMPLES_PASSED);

		// Add to queue
		m_queryQueue.push_back(newQuery);
	}
}

void Octree_CHCPP::AddDepthSortedChildrenToDistanceQueue(OctreeNode* pNode)
{
	// Add all children to an array
	std::vector<NodeAndDistance> children;

	for(int x = 0; x < 2; x++)
		for(int y = 0; y < 2; y++)
			for(int z = 0; z < 2; z++)
			{
				OctreeNode* pChild = &(*pNode->m_children)[x][y][z];
				children.push_back(NodeAndDistance(pChild, (m_cameraPosition - pChild->GetRegion().GetCenter()).Magnitude()));
			}

	// Add in order based on distance from camera
	std::sort(children.begin(), children.end(), &NodeAndDistance::DistCompare);

	// Add to queue
	for(unsigned int i = 0; i < 8; i++)
		m_distanceQueue.push_back(children[i].m_pNode);
}

void Octree_CHCPP::Run(const Vec3f &cameraPosition, const Frustum &frustum)
{
	assert(m_created);

	m_numOccluded = 0;

	m_cameraPosition = cameraPosition;
	m_frustum = frustum;

	// Query outside root elements - add them if they are visible FRUSTUM ONLY
	for(std::unordered_set<OctreeOccupant*>::iterator it = m_pOctree->m_outsideRoot.begin(); it != m_pOctree->m_outsideRoot.end(); it++)
	{
		OctreeOccupant* pOc = *it;

		if(!frustum.Test_AABB_Outside(pOc->GetAABB()))
			m_pOcclusionRenderer->Draw(pOc);
	}

	m_pOcclusionRenderer->FinishDraw();

	// -------------------------------- CHCPP --------------------------------

	m_alreadyRendered.clear();

	m_previousFrameID = m_frameID - 1;

	m_distanceQueue.push_back(m_pOctree->m_pRootNode.get());

	while(!m_distanceQueue.empty() || !m_queryQueue.empty())
	{
		while(!m_queryQueue.empty())
		{
			CHCPP_Query query;

			if(CheckForFinishedQuery(query))
				HandleReturnedQuery(query);
			else if(!m_visibleQueue.empty())// No existing queries finished, issue some new ones for visible nodes
			{
				IssueQuery(m_visibleQueue.front());
				m_visibleQueue.pop_front();
			}
		}

		if(!m_distanceQueue.empty())
		{
			OctreeNode* pNode = m_distanceQueue.front();
			m_distanceQueue.pop_front();

			if(!frustum.Test_AABB_Outside(pNode->m_region))
			{
				if(!WasVisible(pNode))
					QueryPreviouslyInvisibleNode(pNode);
				else
				{
					// If leaf but query still reasonable, issue one
					if(QueryReasonable(pNode))
						m_visibleQueue.push_back(pNode);
					
					TraverseNode(pNode);
				}
			}
		}

		if(m_distanceQueue.empty())
			// Issue remaining query batch
			IssueMultiQueries();
	}

	while(!m_visibleQueue.empty())
	{
		// Remaining previously visible node queries
		OctreeNode* pNode = m_visibleQueue.front();
		m_visibleQueue.pop_front();

		IssueQuery(pNode);
	}

	m_pOcclusionRenderer->FinishDraw();

	m_frameID++;

	// Wrap
	if(m_frameID > m_wrapFrame)
		m_frameID = 0;

	std::cout << m_numOccluded << std::endl;
}

bool Octree_CHCPP::CheckForFinishedQuery(CHCPP_Query &query)
{
	const CHCPP_Query &first = m_queryQueue.front();

	if(first.m_intersectsCamera)
	{
		query = first;

		// Remove
		m_queryQueue.pop_front();

		return true;
	}
	else
	{
		assert(first.m_query != 0);

		int resultAvailable = 0;

		glGetQueryObjectiv(first.m_query, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);

		if(resultAvailable)
		{
			query = first;

			// Remove
			m_queryQueue.pop_front();

			return true;
		}
	}

	return false;

	/*
	for(std::list<CHCPP_Query>::iterator it = m_queryQueue.begin(); it != m_queryQueue.end(); it++)
	{
		if(it->m_intersectsCamera)
		{
			query = *it;

			// Remove
			m_queryQueue.erase(it);

			return true;
		}
		else
		{
			assert(it->m_query != 0);

			int resultAvailable = 0;

			glGetQueryObjectiv(it->m_query, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);

			if(resultAvailable)
			{
				query = *it;

				// Remove
				m_queryQueue.erase(it);

				return true;
			}
		}
	}

	return false;*/
}

bool Octree_CHCPP::QueryReasonable(OctreeNode* pNode)
{
	return pNode->GetNumOccupantsBelow() > m_minQueryNodeOccupantCount;
}

bool Octree_CHCPP::WasVisible(OctreeNode* pNode)
{
	return pNode->m_isVisible && pNode->m_lastVisitedFrameID == m_previousFrameID;
}