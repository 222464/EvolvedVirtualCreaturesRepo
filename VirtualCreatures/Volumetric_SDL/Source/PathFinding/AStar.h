#include <PathFinding/MinHeap.h>

#include <System/Uncopyable.h>

#include <World/World.h>

class HashedNodeStatusPoint3i :
	public Point3i
{
public:
	enum ListStatus
	{
		e_open, e_closed, e_unvisited
	} m_status;

	HashedNodeStatusPoint3i()
		: m_status(e_unvisited)
	{
	}

	HashedNodeStatusPoint3i(const Point3i &other)
		: Point3i(other), m_status(e_unvisited)
	{
	}

	// Custom hash stuff
	size_t operator()(const Point3i &point) const
	{
		return static_cast<size_t>(point.x ^ point.y ^ point.z);
	}
};

struct Node_LowMemory
{
	HashedNodeStatusPoint3i m_position;

	Node_LowMemory* m_pParent;

	float m_gCost;
	float m_fCost;

	Node_LowMemory()
		: m_gCost(0.0f), m_fCost(0.0f), m_pParent(NULL)
	{
	}

	Node_LowMemory(const Point3i &point)
		: m_position(point), m_gCost(0.0f), m_fCost(0.0f), m_pParent(NULL)
	{
	}

	Node_LowMemory(const Point3i &point, float gCost, float fCost)
		: m_position(point), m_gCost(gCost), m_fCost(fCost), m_pParent(NULL)
	{
	}

	float GetDistance(Node_LowMemory* pOther) const;

	// Don't store h cost, can calculate from f and g. Need f though for comparison operator in binary heap

	bool operator>(const Node_LowMemory &other) const
	{
		return m_fCost > other.m_fCost;
	}

	bool operator<(const Node_LowMemory &other) const
	{
		return m_fCost < other.m_fCost;
	}

	bool operator==(const Node_LowMemory &other) const
	{
		return m_fCost == other.m_fCost;
	}

	bool operator!=(const Node_LowMemory &other) const
	{
		return m_fCost != other.m_fCost;
	}
};

class MinHeap_LowMemoryNode :
	public Uncopyable
{
public:
	std::vector<Node_LowMemory*> m_values;

	unsigned int GetSize() const
	{
		return m_values.size();
	}

	bool IsEmpty() const
	{
		return m_values.empty();
	}

	// Returns the array index of the value's final resting place
	int Push(Node_LowMemory* value);

	// Returns the array index of the value's final resting place
	int SiftUp(int index);

	void SiftDown(int index);

	Node_LowMemory* Pop();
};

void GetPath_LowMemory(std::vector<Point3i> &solution, World* pWorld,
	const Point3i &start, const Point3i &end,
	void (*pSuccessorFunc)(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos),
	float (*pHeuristicFunc)(const Point3i &start, const Point3i &end), unsigned int maxSteps);

float HeuristicFunc_ExactDistance(const Point3i &start, const Point3i &end);
float HeuristicFunc_UnderEstimate(const Point3i &start, const Point3i &end);

float HeuristicFunc_ManhattanDistance(const Point3i &start, const Point3i &end);

void AddSuccessorFunc_Flying(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos);
void AddSuccessorFunc_Walking(World* pWorld, std::vector<Node_LowMemory*> &nodes, const Point3i &pos);
