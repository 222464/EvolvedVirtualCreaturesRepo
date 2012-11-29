#pragma once

#include <Constructs/Vec3f.h>

class Plane_Equationf
{
public:
	float a, b, c, d;

	Plane_Equationf();
	Plane_Equationf(float A, float B, float C, float D);
	Plane_Equationf(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);
	Plane_Equationf(const Vec3f &point, const Vec3f &normal);

	void FromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);

	void FromAnchorNormal(const Vec3f &point, const Vec3f &normal);

	void Normalized_FromEquationCoeffs(float A, float B, float C, float D);

	float DistanceTo(const Vec3f &point) const;
	float SignedDistanceTo(const Vec3f &point) const;
};

