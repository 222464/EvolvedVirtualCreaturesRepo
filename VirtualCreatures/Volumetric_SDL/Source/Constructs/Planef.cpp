#include <Constructs/Planef.h>

#include <Utilities/UtilFuncs.h>

Planef::Planef()
{
}

Planef::Planef(const Vec3f &point, const Vec3f &normal)
	: m_point(point), m_normal(normal)
{
}

Planef::Planef(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3)
{
	FromPoints(p1, p2, p3);
}

Planef::Planef(float a, float b, float c, float d)
{
	FromEquationCoeffs(a, b, c, d);
}

void Planef::FromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3)
{
	// Set point to any of the points
	m_point = p1;

	// Calculate the normal
	m_normal = (p2 - p1).Cross(p3 - p1);
}

float Planef::DistanceTo(const Vec3f &point) const
{
	return absf(m_normal.x * (point.x - m_point.x) + m_normal.y * (point.y - m_point.y) + m_normal.z * (point.z - m_point.z)) / m_normal.Magnitude();
}

float Planef::SignedDistanceTo(const Vec3f &point) const
{
	return (m_normal.x * (point.x - m_point.x) + m_normal.y * (point.y - m_point.y) + m_normal.z * (point.z - m_point.z)) / m_normal.Magnitude();
}

void Planef::FromEquationCoeffs(float a, float b, float c, float d)
{
	m_normal.x = a;
	m_normal.y = b;
	m_normal.z = c;

	// Get a point following the equation and set it as the anchor

	// For different possible orientations that produce infinity as a result
	if(a != 0.0f)
	{
		m_point.x = -d / a;
		m_point.y = 0.0f;
		m_point.z = 0.0f;
	}
	else if(b != 0.0f)
	{
		m_point.x = 0.0f;
		m_point.y = -d / b;
		m_point.z = 0.0f;
	}
	else
	{
		m_point.x = 0.0f;
		m_point.y = 0.0f;
		m_point.z = -d / c;
	}
}