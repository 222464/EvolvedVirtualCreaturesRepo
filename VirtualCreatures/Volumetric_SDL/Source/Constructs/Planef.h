#pragma once

#include <Constructs/Vec3f.h>

class Planef
{
public:
	Vec3f m_point;
	Vec3f m_normal;

	Planef();
	Planef(const Vec3f &point, const Vec3f &normal);
	Planef(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);
	Planef(float a, float b, float c, float d);

	void FromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);

	void FromEquationCoeffs(float a, float b, float c, float d);

	float DistanceTo(const Vec3f &point) const;
	float SignedDistanceTo(const Vec3f &point) const;
};

