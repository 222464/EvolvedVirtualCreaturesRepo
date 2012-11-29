#include <Constructs/AABB.h>

#include <Renderer/SDL_OpenGL.h>

#include <Constructs/Quaternion.h>

#include <limits>

#include <assert.h>

AABB::AABB()
	: m_lowerBound(0.0f, 0.0f, 0.0f), m_upperBound(1.0f, 1.0f, 1.0f),
	m_center(0.5f, 0.5f, 0.5f), m_halfDims(0.5f, 0.5f, 0.5f)
{
}

AABB::AABB(const Vec3f &lowerBound, const Vec3f &upperBound)
	: m_lowerBound(lowerBound), m_upperBound(upperBound)
{
	CalculateHalfDims();
	CalculateCenter();
}

void AABB::CalculateHalfDims()
{
	m_halfDims = (m_upperBound - m_lowerBound) / 2.0f;
}

void AABB::CalculateCenter()
{
	m_center = m_lowerBound + m_halfDims;
}

void AABB::CalculateBounds()
{
	m_lowerBound = m_center - m_halfDims;
	m_upperBound = m_center + m_halfDims;
}

const Vec3f &AABB::GetCenter() const
{
	return m_center;
}

Vec3f AABB::GetDims() const
{
	return m_upperBound - m_lowerBound;
}

const Vec3f &AABB::GetHalfDims() const
{
	return m_halfDims;
}

const Vec3f &AABB::GetLowerBound() const
{
	return m_lowerBound;
}

const Vec3f &AABB::GetUpperBound() const
{
	return m_upperBound;
}

void AABB::SetCenter(const Vec3f &center)
{
	m_center = center;
	
	CalculateBounds();
}

void AABB::IncCenter(const Vec3f &increment)
{
	m_center += increment;
	
	CalculateBounds();
}

void AABB::SetDims(const Vec3f &dims)
{
	SetHalfDims(dims / 2.0f);
}

void AABB::SetHalfDims(const Vec3f &halfDims)
{
	m_halfDims = halfDims;

	CalculateBounds();
}

void AABB::Scale(const Vec3f &scale)
{
	SetHalfDims(m_halfDims * scale);
}

bool AABB::Intersects(const AABB &other) const
{
	if(m_upperBound.x < other.m_lowerBound.x)
		return false;

	if(m_upperBound.y < other.m_lowerBound.y)
		return false;

	if(m_upperBound.z < other.m_lowerBound.z)
		return false;

	if(m_lowerBound.x > other.m_upperBound.x)
		return false;

	if(m_lowerBound.y > other.m_upperBound.y)
		return false;

	if(m_lowerBound.z > other.m_upperBound.z)
		return false;

	return true;
}

bool AABB::Intersects(const Vec3f& p1, const Vec3f& p2) const
{
    Vec3f d((p2 - p1) / 2.0f);
    Vec3f e((m_upperBound - m_lowerBound) / 2.0f);
    Vec3f c(p1 + d - (m_lowerBound + m_upperBound) / 2.0f);
    Vec3f ad(fabsf(d.x), fabsf(d.y), fabsf(d.z)); // Returns same vector with all components positive

    if (fabsf(c.x) > e.x + ad.x)
        return false;
    if (fabsf(c.y) > e.y + ad.y)
        return false;
    if (fabsf(c.z) > e.z + ad.z)
        return false;

	float epsilon = std::numeric_limits<float>::epsilon();
  
    if(fabsf(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + epsilon)
        return false;
    if(fabsf(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + epsilon)
        return false;
    if(fabsf(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + epsilon)
        return false;
            
    return true;
}

bool AABB::Intersects(const Vec3f &start, const Vec3f &direction, float t[2]) const
{
	int parallel = 0;
	bool found = false;

	float epsilon = std::numeric_limits<float>::epsilon();

	// Array representing vector so it is easier to parse
	float S[3] = {start.x, start.y, start.z};
	float D[3] = {direction.x, direction.y, direction.z};
	float d[3] = {m_center.x - start.x, m_center.y - start.y, m_center.z - start.z};
	float e[3] = {m_halfDims.x, m_halfDims.y, m_halfDims.z};

	for(int i = 0; i < 3; i++)
	{
		if(fabsf(D[i]) < epsilon)
			parallel |= 1 << i;
		else
		{
			float es = D[i] > 0.0f ? e[i] : -e[i];

			if(!found)
			{
				t[0] = (d[i] - es) / D[i];
				t[1] = (d[i] + es) / D[i];

				found = true;
			}
			else
			{
				float s = (d[i] - es) / D[i];

				if(s > t[0])
					t[0] = s;

				s = (d[i] + es) / D[i];

				if(s < t[1])
					t[1] = s;

				if(t[0] > t[1])
					return false;
			}
		}
	}

	if(parallel)
		for(int i = 0; i < 3; i++)
			if(parallel & (1 << i))
				if(fabsf(d[i] - t[0] * D[i]) > e[i] || fabsf(d[i] - t[1] * D[i]) > e[i])
					return false;

	return true;
}

bool AABB::Contains(const AABB &other) const
{
	if(other.m_lowerBound.x >= m_lowerBound.x &&
		other.m_upperBound.x <= m_upperBound.x &&
		other.m_lowerBound.y >= m_lowerBound.y &&
		other.m_upperBound.y <= m_upperBound.y &&
		other.m_lowerBound.z >= m_lowerBound.z &&
		other.m_upperBound.z <= m_upperBound.z)
		return true;

	return false;
}

bool AABB::Contains(const Vec3f &vec) const
{
	if(vec.x >= m_lowerBound.x &&
		vec.y >= m_lowerBound.y &&
		vec.z >= m_lowerBound.z &&
		vec.x <= m_upperBound.x && 
		vec.y <= m_upperBound.y &&
		vec.z <= m_upperBound.z)
		return true;

	return false;
}

AABB AABB::GetTransformedAABB(const Matrix4x4f &mat) const
{
	AABB transformedAABB;

	Vec3f newCenter(mat * m_center);

	transformedAABB.m_lowerBound = newCenter;
	transformedAABB.m_upperBound = newCenter;

	// Loop through all corners, transform, and compare
	for(int x = -1; x <= 1; x += 2)
		for(int y = -1; y <= 1; y += 2)
			for(int z = -1; z <= 1; z += 2)
			{
				Vec3f corner(x * m_halfDims.x + m_center.x, y * m_halfDims.y + m_center.y, z * m_halfDims.z + m_center.z);

				// Transform the corner
				corner = mat * corner;

				// Compare bounds
				if(corner.x > transformedAABB.m_upperBound.x)
					transformedAABB.m_upperBound.x = corner.x;
				if(corner.y > transformedAABB.m_upperBound.y)
					transformedAABB.m_upperBound.y = corner.y;
				if(corner.z > transformedAABB.m_upperBound.z)
					transformedAABB.m_upperBound.z = corner.z;

				if(corner.x < transformedAABB.m_lowerBound.x)
					transformedAABB.m_lowerBound.x = corner.x;
				if(corner.y < transformedAABB.m_lowerBound.y)
					transformedAABB.m_lowerBound.y = corner.y;
				if(corner.z < transformedAABB.m_lowerBound.z)
					transformedAABB.m_lowerBound.z = corner.z;
			}

	// Move from local into world space
	transformedAABB.CalculateHalfDims();
	transformedAABB.CalculateCenter();

	return transformedAABB;
}

void AABB::DebugRender() const
{
	// Render the AABB with lines
	glBegin(GL_LINES);

	// Side 1
	
	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_lowerBound.z);
	glVertex3f(m_upperBound.x, m_lowerBound.y, m_lowerBound.z);

	glVertex3f(m_lowerBound.x, m_upperBound.y, m_lowerBound.z);
	glVertex3f(m_upperBound.x, m_upperBound.y, m_lowerBound.z);

	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_lowerBound.z);
	glVertex3f(m_lowerBound.x, m_upperBound.y, m_lowerBound.z);

	glVertex3f(m_upperBound.x, m_lowerBound.y, m_lowerBound.z);
	glVertex3f(m_upperBound.x, m_upperBound.y, m_lowerBound.z);

	// Side 2

	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_upperBound.z);
	glVertex3f(m_upperBound.x, m_lowerBound.y, m_upperBound.z);

	glVertex3f(m_lowerBound.x, m_upperBound.y, m_upperBound.z);
	glVertex3f(m_upperBound.x, m_upperBound.y, m_upperBound.z);

	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_upperBound.z);
	glVertex3f(m_lowerBound.x, m_upperBound.y, m_upperBound.z);

	glVertex3f(m_upperBound.x, m_lowerBound.y, m_upperBound.z);
	glVertex3f(m_upperBound.x, m_upperBound.y, m_upperBound.z);

	// Connections

	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_lowerBound.z);
	glVertex3f(m_lowerBound.x, m_lowerBound.y, m_upperBound.z);

	glVertex3f(m_upperBound.x, m_lowerBound.y, m_lowerBound.z);
	glVertex3f(m_upperBound.x, m_lowerBound.y, m_upperBound.z);

	glVertex3f(m_upperBound.x, m_upperBound.y, m_lowerBound.z);
	glVertex3f(m_upperBound.x, m_upperBound.y, m_upperBound.z);
	
	glVertex3f(m_lowerBound.x, m_upperBound.y, m_lowerBound.z);
	glVertex3f(m_lowerBound.x, m_upperBound.y, m_upperBound.z);

	glEnd();
}

Vec3f AABB::GetVertexP(const Vec3f &normal) const
{
	Vec3f p(m_lowerBound);

	if(normal.x >= 0.0f)
		p.x = m_upperBound.x;
	if(normal.y >= 0.0f)
		p.y = m_upperBound.y;
	if(normal.z >= 0.0f)
		p.z = m_upperBound.z;

	return p;
}

Vec3f AABB::GetVertexN(const Vec3f &normal) const
{
	Vec3f n(m_upperBound);

	if(normal.x >= 0.0f)
		n.x = m_lowerBound.x;
	if(normal.y >= 0.0f)
		n.y = m_lowerBound.y;
	if(normal.z >= 0.0f)
		n.z = m_lowerBound.z;

	return n;
}

float AABB::GetRadius() const
{
	return m_halfDims.Magnitude();
}