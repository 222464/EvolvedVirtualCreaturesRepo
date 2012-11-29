#include <Renderer/Octree/Octree_CHC.h>

#include <Renderer/Octree/OctreeNode.h>
#include <Renderer/Octree/Octree.h>

#include <Renderer/Culler/Frustum.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

#include <algorithm>

Octree_CHC::CHC_Query::CHC_Query()
	: m_query(0), m_intersectsCamera(false), m_pNode(NULL)
{
}

Octree_CHC::NodeAndDistance::NodeAndDistance()
{
}

Octree_CHC::NodeAndDistance::NodeAndDistance(OctreeNode* pNode, float distance)
	: m_pNode(pNode), m_distance(distance)
{
}

bool Octree_CHC::NodeAndDistance::DistCompare(const NodeAndDistance &first, const NodeAndDistance &second)
{
	return first.m_distance < second.m_distance;
}

Octree_CHC::Octree_CHC()
	: m_created(false),
	m_frameID(0), m_wrapFrame(4000)
{
}

Octree_CHC::~Octree_CHC()
{
	// Delete left over queries
	for(std::list<CHC_Query>::iterator it = m_queryQueue.begin(); it != m_queryQueue.end(); it++)
		glDeleteQueries(1, &(*it).m_query);
}

void Octree_CHC::Create(Octree* pOctree, Occlusion_Renderer* pCHCRenderer)
{
	m_pOctree = pOctree;
	m_pOcclusionRenderer = pCHCRenderer;

	m_created = true;
}

void Octree_CHC::HandleReturnedQuery(const CHC_Query &query)
{
	assert(query.m_pNode != NULL);

	if(query.m_intersectsCamera)
	{
		TraverseNode(query.m_pNode);

		PullUpVisibility(query.m_pNode);
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
			TraverseNode(query.m_pNode);

			PullUpVisibility(query.m_pNode);
		}
		else
			m_numOccluded++;
	}
}

void Octree_CHC::TraverseNode(OctreeNode* pNode)
{
	if(pNode->m_hasChildren)
	{
		AddDepthSortedChildrenToDistanceQueue(pNode);
		pNode->m_isVisible = false;
	}
	
	// Draw all occupants of node
	for(std::unordered_set<OctreeOccupant*>::iterator it = pNode->m_pOccupants.begin(); it != pNode->m_pOccupants.end(); it++)
	{
		OctreeOccupant* pOccupant = *it;

		std::unordered_set<OctreeOccupant*>::iterator it_find = m_alreadyRendered.find(pOccupant);

		if(it_find == m_alreadyRendered.end())
		{
			// If in the frustum
			if(!m_frustum.Test_AABB_Outside(pOccupant->GetAABB()))
				m_pOcclusionRenderer->Draw(pOccupant);

			m_alreadyRendered.insert(pOccupant);
		}
	}
}

void Octree_CHC::PullUpVisibility(OctreeNode* pNode)
{
	while(pNode != NULL && !pNode->m_isVisible)
	{
		pNode->m_isVisible = true;
		pNode = pNode->m_pParent;
	}
}

void Octree_CHC::IssueQuery(OctreeNode* pNode)
{
	// If it intersects the bounding volume, no query is necessary
	if(pNode->GetRegion().Contains(m_cameraPosition))
	{
		CHC_Query newQuery;

		newQuery.m_pNode = pNode;

		newQuery.m_intersectsCamera = true;

		// Add to queue
		m_queryQueue.push_back(newQuery);
	}
	else
	{
		// Make sure rendering is done before query is assigned
		m_pOcclusionRenderer->FinishDraw();

		CHC_Query newQuery;

		newQuery.m_pNode = pNode;

		glGenQueries(1, &newQuery.m_query);

		glBeginQuery(GL_ANY_SAMPLES_PASSED, newQuery.m_query);

		m_pOcclusionRenderer->DrawAABB(pNode->GetRegion());

		glEndQuery(GL_ANY_SAMPLES_PASSED);

		// Add to queue
		m_queryQueue.push_back(newQuery);
	}
}

void Octree_CHC::AddDepthSortedChildrenToDistanceQueue(OctreeNode* pNode)
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

void Octree_CHC::Run(const Vec3f &cameraPosition, const Frustum &frustum)
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

	// -------------------------------- CHC --------------------------------

	m_alreadyRendered.clear();

	m_distanceQueue.push_back(m_pOctree->m_pRootNode.get());

	m_previousFrameID = m_frameID - 1;

	// Wrap
	if(m_previousFrameID < 0)
		m_previousFrameID = m_wrapFrame;

	while(!m_distanceQueue.empty() || !m_queryQueue.empty())
	{
		while(!m_queryQueue.empty() && (CheckFirstQueryFinished() || m_distanceQueue.empty()))
		{
			CHC_Query query(m_queryQueue.front());
			m_queryQueue.pop_front();
			HandleReturnedQuery(query);
		}

		if(!m_distanceQueue.empty())
		{
			OctreeNode* pNode = m_distanceQueue.front();
			m_distanceQueue.pop_front();

			if(!frustum.Test_AABB_Outside(pNode->m_region))
			{
				bool wasVisible = pNode->m_isVisible && pNode->m_lastVisitedFrameID == m_previousFrameID;

				bool isLeafOrWasInvisible = !pNode->m_hasChildren || !wasVisible;

				// Update node's visibilty information
				pNode->m_isVisible = false;
				pNode->m_lastVisitedFrameID = m_frameID;

				if(isLeafOrWasInvisible)
					IssueQuery(pNode);

				if(wasVisible)
					TraverseNode(pNode);
			}
		}
	}

	m_pOcclusionRenderer->FinishDraw();

	m_frameID++;

	// Wrap
	if(m_frameID > m_wrapFrame)
		m_frameID = 0;

	std::cout << m_numOccluded << std::endl;
}

bool Octree_CHC::CheckFirstQueryFinished()
{
	const CHC_Query &first = m_queryQueue.front();

	if(first.m_intersectsCamera)
		return true;
	else
	{
		assert(first.m_query != 0);

		int resultAvailable = 0;

		glGetQueryObjectiv(first.m_query, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);

		if(resultAvailable)
			return true;
	}

	return false;
}