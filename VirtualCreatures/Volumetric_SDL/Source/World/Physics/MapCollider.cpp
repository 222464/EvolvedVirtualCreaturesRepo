#include <World/Physics/MapCollider.h>

#include <assert.h>

MapCollider::MapCollider()
	: m_pWorld(NULL),
	m_center(0.0f, 0.0f, 0.0f), m_vel(0.0f, 0.0f, 0.0f), m_halfDims(0.3f, 0.3f, 0.3f)
{
}

bool MapCollider::Update()
{
	assert(m_pWorld != NULL);

	bool hitGroundInY = false;

	// Check 6 points around for collision with map
	if(m_pWorld->GetVoxel(static_cast<int>(m_center.x - m_halfDims.x), static_cast<int>(m_center.y), static_cast<int>(m_center.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.x += static_cast<float>(static_cast<int>(m_center.x - m_halfDims.x)) + 1.0f - (m_center.x - m_halfDims.x);
		m_vel.x = 0.0f;
	}
	else if(m_pWorld->GetVoxel(static_cast<int>(m_center.x + m_halfDims.x), static_cast<int>(m_center.y), static_cast<int>(m_center.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.x += static_cast<float>(static_cast<int>(m_center.x + m_halfDims.x)) - (m_center.x + m_halfDims.x);
		m_vel.x = 0.0f;
	}

	if(m_pWorld->GetVoxel(static_cast<int>(m_center.x), static_cast<int>(m_center.y - m_halfDims.y), static_cast<int>(m_center.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.y += static_cast<float>(static_cast<int>(m_center.y - m_halfDims.y)) + 1.0f - (m_center.y - m_halfDims.y);
		m_vel.y = 0.0f;

		hitGroundInY = true;
	}
	else if(m_pWorld->GetVoxel(static_cast<int>(m_center.x), static_cast<int>(m_center.y + m_halfDims.y), static_cast<int>(m_center.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.y += static_cast<float>(static_cast<int>(m_center.y + m_halfDims.y)) - (m_center.y + m_halfDims.y);
		m_vel.y = 0.0f;
	}

	if(m_pWorld->GetVoxel(static_cast<int>(m_center.x), static_cast<int>(m_center.y), static_cast<int>(m_center.z - m_halfDims.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.z += static_cast<float>(static_cast<int>(m_center.z - m_halfDims.z)) + 1.0f - (m_center.z - m_halfDims.z);
		m_vel.z = 0.0f;
	}
	else if(m_pWorld->GetVoxel(static_cast<int>(m_center.x), static_cast<int>(m_center.y), static_cast<int>(m_center.z + m_halfDims.z)) != 0)
	{
		// Hit, move to nearest higher whole voxel position in the x direction
		m_center.z += static_cast<float>(static_cast<int>(m_center.z + m_halfDims.z)) - (m_center.z + m_halfDims.z);
		m_vel.z = 0.0f;
	}

	return hitGroundInY;
}
