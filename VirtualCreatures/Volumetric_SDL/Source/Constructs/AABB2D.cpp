#include <Constructs/AABB2D.h>

#include <Renderer/SDL_OpenGL.h>

#include <Constructs/Quaternion.h>

#include <limits>

#include <assert.h>

AABB2D::AABB2D()
	: m_lowerBound(0.0f, 0.0f), m_upperBound(1.0f, 1.0f),
	m_center(0.5f, 0.5f), m_halfDims(0.5f, 0.5f)
{
}

AABB2D::AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound)
	: m_lowerBound(lowerBound), m_upperBound(upperBound)
{
	CalculateHalfDims();
	CalculateCenter();
}

void AABB2D::CalculateHalfDims()
{
	m_halfDims = (m_upperBound - m_lowerBound) / 2.0f;
}

void AABB2D::CalculateCenter()
{
	m_center = m_lowerBound + m_halfDims;
}

void AABB2D::CalculateBounds()
{
	m_lowerBound = m_center - m_halfDims;
	m_upperBound = m_center + m_halfDims;
}

const Vec2f &AABB2D::GetCenter() const
{
	return m_center;
}

Vec2f AABB2D::GetDims() const
{
	return m_upperBound - m_lowerBound;
}

const Vec2f &AABB2D::GetHalfDims() const
{
	return m_halfDims;
}

const Vec2f &AABB2D::GetLowerBound() const
{
	return m_lowerBound;
}

const Vec2f &AABB2D::GetUpperBound() const
{
	return m_upperBound;
}

void AABB2D::SetCenter(const Vec2f &center)
{
	m_center = center;
	
	CalculateBounds();
}

void AABB2D::IncCenter(const Vec2f &increment)
{
	m_center += increment;
	
	CalculateBounds();
}

void AABB2D::SetDims(const Vec2f &dims)
{
	SetHalfDims(dims / 2.0f);
}

void AABB2D::SetHalfDims(const Vec2f &halfDims)
{
	m_halfDims = halfDims;

	CalculateBounds();
}

void AABB2D::Scale(const Vec2f &scale)
{
	SetHalfDims(m_halfDims * scale);
}

bool AABB2D::Intersects(const AABB2D &other) const
{
	if(m_upperBound.x < other.m_lowerBound.x)
		return false;

	if(m_upperBound.y < other.m_lowerBound.y)
		return false;

	if(m_lowerBound.x > other.m_upperBound.x)
		return false;

	if(m_lowerBound.y > other.m_upperBound.y)
		return false;

	return true;
}

bool AABB2D::Contains(const AABB2D &other) const
{
	if(other.m_lowerBound.x >= m_lowerBound.x &&
		other.m_upperBound.x <= m_upperBound.x &&
		other.m_lowerBound.y >= m_lowerBound.y &&
		other.m_upperBound.y <= m_upperBound.y)
		return true;

	return false;
}

bool AABB2D::Contains(const Vec2f &vec) const
{
	if(vec.x >= m_lowerBound.x &&
		vec.y >= m_lowerBound.y &&
		vec.x <= m_upperBound.x && 
		vec.y <= m_upperBound.y)
		return true;

	return false;
}

AABB2D AABB2D::GetTransformedAABB2D(const Matrix4x4f &mat) const
{
	AABB2D transformedAABB2D;

	Vec3f newCenter(mat * Vec3f(m_center.x, m_center.y, 0.0f));

	transformedAABB2D.m_lowerBound.x = transformedAABB2D.m_upperBound.x = newCenter.x;
	transformedAABB2D.m_lowerBound.y = transformedAABB2D.m_upperBound.y = newCenter.y;

	// Loop through all corners, transform, and compare
	for(int x = -1; x <= 1; x += 2)
		for(int y = -1; y <= 1; y += 2)
			for(int z = -1; z <= 1; z += 2)
			{
				Vec3f corner(x * m_halfDims.x + m_center.x, y * m_halfDims.y + m_center.y, 0.0f);

				// Transform the corner
				corner = mat * corner;

				// Compare bounds
				if(corner.x > transformedAABB2D.m_upperBound.x)
					transformedAABB2D.m_upperBound.x = corner.x;
				if(corner.y > transformedAABB2D.m_upperBound.y)
					transformedAABB2D.m_upperBound.y = corner.y;

				if(corner.x < transformedAABB2D.m_lowerBound.x)
					transformedAABB2D.m_lowerBound.x = corner.x;
				if(corner.y < transformedAABB2D.m_lowerBound.y)
					transformedAABB2D.m_lowerBound.y = corner.y;
			}

	// Move from local into world space
	transformedAABB2D.CalculateHalfDims();
	transformedAABB2D.CalculateCenter();

	return transformedAABB2D;
}

float AABB2D::GetRadius() const
{
	return m_halfDims.Magnitude();
}