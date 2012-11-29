#include <PathFinding/AStar.h>

#include <World/World.h>

const unsigned char s_visitedVoxelID = 254;
const unsigned char s_openListVoxelID = 255;

float Node_LowMemory::GetDistance(Node_LowMemory* pOther) const
{
	int numAxisOffsets = abs(m_position.x -  pOther->m_position.x) +
		abs(m_position.y -  pOther->m_position.y) +
		abs(m_position.z -  pOther->m_position.z);

	return numAxisOffsets == 1 ? 1.0f : (numAxisOffsets == 2 ? 1.4f : 1.7f);

	/*int x2 = m_position.x - pOther->m_position.x;
	x2 *= x2;

	int y2 = m_position.y - pOther->m_position.y;
	y2 *= y2;

	int z2 = m_position.z - pOther->m_position.z;
	z2 *= z2;

	return sqrtf(static_cast<float>(x2 + y2 + z2));*/
}

int MinHeap_LowMemoryNode::Push(Node_LowMemory* value)
{
	m_values.push_back(value);

	return SiftUp(m_values.size() - 1);
}

int MinHeap_LowMemoryNode::SiftUp(int index)
{
	while(true)
	{
		// Get the parent of the value at the index
		int parentIndex = static_cast<int>((index - 1) / 2.0f); // Floored by casting to an int
			
		// Check to see that the index exists
		if(parentIndex < 0)
			break;
			
		// Swap if the parent value is greater than the current value
		if(m_values[index]->m_fCost < m_values[parentIndex]->m_fCost)
			std::swap(m_values[index], m_values[parentIndex]);
		else
			break;
			
		index = parentIndex;
	}
		
	return index;
}

void MinHeap_LowMemoryNode::SiftDown(int index)
{
	const int numValues = m_values.size();
		
	while(true)
	{
		Node_LowMemory* indexValue = m_values[index];
			
		// Calculate child indices based on properties given previously
		int firstChildIndex = 2 * index + 1;
		int secondChildIndex = firstChildIndex + 1;
			
		// Alter behavior based on children existence
		if(firstChildIndex < numValues)
		{
			Node_LowMemory* firstChildValue = m_values[firstChildIndex];
				
			if(secondChildIndex < numValues)
			{
				Node_LowMemory* secondChildValue = m_values[secondChildIndex];
					
				// Check that the value at index is less than both children
				if(indexValue->m_fCost > firstChildValue->m_fCost || indexValue->m_fCost > secondChildValue->m_fCost)
				{
					// Parent is greater than one of the children, swap it
					// with the smaller one and sift downwards from there
					if(firstChildValue->m_fCost < secondChildValue->m_fCost)
					{
						// Swap
						m_values[index] = firstChildValue;
						m_values[firstChildIndex] = indexValue;
							
						index = firstChildIndex;
					}
					else
					{
						// Swap
						m_values[index] = secondChildValue;
						m_values[secondChildIndex] = indexValue;
							
						index = secondChildIndex;
					}
				}
				else
					return;
			}
			else
			{
				// Compare the index value to the only child value. If it is greater than its child, swap them
				if(indexValue->m_fCost > firstChildValue->m_fCost)
				{
					m_values[index] = firstChildValue;
					m_values[firstChildIndex] = indexValue;
						
					index = firstChildIndex;
				}
				else
					return;
			}
		}
		else
			return;
	}
}

Node_LowMemory* MinHeap_LowMemoryNode::Pop()
{
	// Check to see that there are values to pop
	if(m_values.empty())
		return NULL;
		
	Node_LowMemory* first = m_values[0];
			
	// Get last value and remove it
	Node_LowMemory* last = m_values.back();
	m_values.pop_back();
		
	if(!m_values.empty())
	{
		// Set the first element to the last one
		m_values[0] = last;
			
		// Sift downwards to re-balance the tree
		SiftDown(0);
	}
		
	return first;
}

void GetPath_LowMemory(std::vector<Point3i> &solution, World* pWorld,
	const Point3i &start, const Point3i &end,
	void (*pSuccessorFunc)(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos),
	float (*pHeuristicFunc)(const Point3i &start, const Point3i &end), unsigned int maxSteps)
{
	// Set of all explored nodes, with custom hash
	std::vector<Node_LowMemory*> allExplored;

	// For linking the map to list statuses without modifying the map
	std::unordered_set<HashedNodeStatusPoint3i, HashedNodeStatusPoint3i> listStatus;

	MinHeap_LowMemoryNode openList;

	Node_LowMemory* startNode = new Node_LowMemory(start);

	allExplored.push_back(startNode);

	openList.Push(startNode);

	startNode->m_position.m_status = HashedNodeStatusPoint3i::e_open;

	listStatus.insert(startNode->m_position);

	Node_LowMemory* pCurrent = startNode;

	bool pathFound = false;

	unsigned int steps = 0;

	while(!openList.IsEmpty() && steps < maxSteps)
	{
		// If found goal, stop
		if(pCurrent->m_position == end)
		{
			pathFound = true;

			break;
		}

		steps++;

		// Get lowest cost node
		pCurrent = openList.Pop();

		// Add to closed list
		pCurrent->m_position.m_status = HashedNodeStatusPoint3i::e_closed;

		listStatus.insert(pCurrent->m_position);

		// Get successors for the node
		std::vector<Node_LowMemory*> newSuccessors;

		(*pSuccessorFunc)(pWorld, newSuccessors, pCurrent->m_position);

		// Go through successors
		for(unsigned int i = 0, size = newSuccessors.size(); i < size; i++)
		{
			Node_LowMemory* pNewSuccessor = newSuccessors[i];

			// If not in list status list, it is unvisited
			std::unordered_set<HashedNodeStatusPoint3i, HashedNodeStatusPoint3i>::iterator it = listStatus.find(pNewSuccessor->m_position);

			// If unvisited
			if(it == listStatus.end())
			{
				// Cost calculations
				pNewSuccessor->m_gCost = pCurrent->m_gCost + pCurrent->GetDistance(pNewSuccessor);
				pNewSuccessor->m_fCost = pNewSuccessor->m_gCost + (*pHeuristicFunc)(pNewSuccessor->m_position, end);

				pNewSuccessor->m_pParent = pCurrent;

				allExplored.push_back(pNewSuccessor);

				openList.Push(pNewSuccessor);

				pNewSuccessor->m_position.m_status = HashedNodeStatusPoint3i::e_open;

				listStatus.insert(pNewSuccessor->m_position);

				pWorld->SetVoxel_NoCheck(pNewSuccessor->m_position.x, pNewSuccessor->m_position.y, pNewSuccessor->m_position.z, s_openListVoxelID);
			}
			else if(it->m_status == HashedNodeStatusPoint3i::e_open) // If already on open list
			{
				// Compare to existing path, see which one is better
				float new_gCost = pCurrent->m_gCost + pCurrent->GetDistance(pNewSuccessor);

				if(new_gCost < pNewSuccessor->m_gCost)
				{
					// New path is better, recalculate costs and parent
					float oldH = pNewSuccessor->m_fCost - pNewSuccessor->m_gCost;
					pNewSuccessor->m_gCost = new_gCost;
					pNewSuccessor->m_fCost = oldH + new_gCost;

					pNewSuccessor->m_pParent = pCurrent;

					// Since a cost was changed, have to resort the binary heap (sift up, since it was changed to something lower)
					// However, to do so, must first find the position in the internal heap array of this node from which to start sifting
					int i = 0;
						
					for(int size = openList.GetSize(); i < size; i++)
					{
						// Look for the node simply based on whether or not the references are equal
						if(openList.m_values[i] == pNewSuccessor)
							break;
					}
						
					// Sift starting from the index of the element just found
					openList.SiftUp(i);
				}
			}
			else // Must be on closed list, delete and ignore
			{
				assert(it->m_status == HashedNodeStatusPoint3i::e_closed);

				delete pNewSuccessor;
			}
		}
	}

	// If path was found, reconstruct solution by following parent node pointers
	if(pathFound)
	{
		// For convenience, already add the end
		solution.push_back(end);

		// pCurrent is pointing to the node before the end in the solution when the search has completed
		for(; pCurrent != NULL; pCurrent = pCurrent->m_pParent)
			solution.push_back(pCurrent->m_position);
	}

	// Delete everything in the all visited set
	for(unsigned int i = 0, size = allExplored.size(); i < size; i++)
	{
		const Point3i &pos = allExplored[i]->m_position;

		// Reset voxel and delete node
		pWorld->SetVoxel_NoCheck(pos.x, pos.y, pos.z, 0);

		delete allExplored[i];
	}
}

float HeuristicFunc_ExactDistance(const Point3i &start, const Point3i &end)
{
	int x2 = start.x - end.x;
	x2 *= x2;

	int y2 = start.y - end.y;
	y2 *= y2;

	int z2 = start.z - end.z;
	z2 *= z2;

	return sqrtf(static_cast<float>(x2 + y2 + z2));
}

float HeuristicFunc_UnderEstimate(const Point3i &start, const Point3i &end)
{
	int x2 = start.x - end.x;
	x2 *= x2;

	int y2 = start.y - end.y;
	y2 *= y2;

	int z2 = start.z - end.z;
	z2 *= z2;

	return sqrtf(static_cast<float>(x2 + y2 + z2)) / 4.0f;
}

float HeuristicFunc_ManhattanDistance(const Point3i &start, const Point3i &end)
{
	return static_cast<float>((abs(start.x - end.x) + abs(start.y - end.y) + abs(start.z - end.z)));
}

void AddSuccessorFunc_Flying(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos)
{
	// Add all surround positions, if they are open
	for(int dx = -1; dx <= 1; dx++)
		for(int dy = -1; dy <= 1; dy++)
			for(int dz = -1; dz <= 1; dz++)
			{
				if(dx == 0 && dy == 0 && dz == 0)
					continue;

				Point3i newPos(pos.x + dx, pos.y + dy, pos.z + dz);

				if(pWorld->GetVoxel(newPos.x, newPos.y, newPos.z) == 0) // Empty, can add successor
					nodes.push_back(new Node_LowMemory(newPos));
			}
}

void AddSuccessorFunc_Walking(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos)
{
	// Add all surround positions, if they are open
	for(int dx = -1; dx <= 1; dx++)
		for(int dy = -1; dy <= 1; dy++)
			for(int dz = -1; dz <= 1; dz++)
			{
				if(dx == 0 && dy == 0 && dz == 0)
					continue;

				Point3i newPos(pos.x + dx, pos.y + dy, pos.z + dz);

				if(pWorld->GetVoxel(newPos.x, newPos.y, newPos.z) == 0)
				{
					if(dy < 0)
						nodes.push_back(new Node_LowMemory(newPos));
					else
					{
						unsigned char voxelID = pWorld->GetVoxel(newPos.x, newPos.y - 1, newPos.z);

						if(voxelID != 0 && voxelID != s_visitedVoxelID && voxelID != s_visitedVoxelID && voxelID != s_openListVoxelID)
							nodes.push_back(new Node_LowMemory(newPos));
					}
				}
			}
}