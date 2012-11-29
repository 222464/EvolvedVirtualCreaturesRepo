#include <Constructs/Plane_Equationf.h>

#include <Utilities/UtilFuncs.h>

Plane_Equationf::Plane_Equationf()
{
}

Plane_Equationf::Plane_Equationf(float A, float B, float C, float D)
	: a(A), b(B), c(C), d(D)
{
}

Plane_Equationf::Plane_Equationf(const Vec3f &point, const Vec3f &normal)
{
	FromAnchorNormal(point, normal);
}

Plane_Equationf::Plane_Equationf(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3)
{
	FromPoints(p1, p2, p3);
}

void Plane_Equationf::FromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3)
{
	// Set point to any of the points
	Vec3f point = p1;

	// Calculate the normal
	Vec3f normal = (p2 - p1).Cross(p3 - p1);

	a = normal.x;
	b = normal.y;
	c = normal.z;

	d = -(a * point.x + b * point.y + c * point.z);
}

void Plane_Equationf::FromAnchorNormal(const Vec3f &point, const Vec3f &normal)
{
	a = normal.x;
	b = normal.y;
	c = normal.z;

	d = -(a * point.x + b * point.y + c * point.z);
}

void Plane_Equationf::Normalized_FromEquationCoeffs(float A, float B, float C, float D)
{
	float mag = sqrtf(A * A + B * B + C * C);

	a = A / mag;
	b = B / mag;
	c = C / mag;
	d = D / mag;
}

float Plane_Equationf::DistanceTo(const Vec3f &point) const
{
	return absf(a * point.x + b * point.y + c * point.z + d) / sqrtf(a * a + b * b + c * c);
}

float Plane_Equationf::SignedDistanceTo(const Vec3f &point) const
{
	return (a * point.x + b * point.y + c * point.z + d) / sqrtf(a * a + b * b + c * c);
}