#pragma once

#include <iostream>

class Vec3f 
{
public:
	float x, y, z;

	Vec3f();
	Vec3f(float X, float Y, float Z);
		
	Vec3f operator*(float scale) const;
	Vec3f operator/(float scale) const;
	Vec3f operator+(const Vec3f &other) const;
	Vec3f operator-(const Vec3f &other) const;
	Vec3f operator*(const Vec3f &other) const;
	Vec3f operator/(const Vec3f &other) const;
	Vec3f operator-() const;
		
	const Vec3f &operator*=(float scale);
	const Vec3f &operator/=(float scale);
	const Vec3f &operator+=(const Vec3f &other);
	const Vec3f &operator-=(const Vec3f &other);
	const Vec3f &operator*=(const Vec3f &other);
	const Vec3f &operator/=(const Vec3f &other);

	bool operator==(const Vec3f &other) const;
	bool operator!=(const Vec3f &other) const;
		
	float Magnitude() const;
	float MagnitudeSquared() const;
	void NormalizeThis();
	Vec3f Normalize() const;
	float Dot(const Vec3f &other) const;
	Vec3f Cross(const Vec3f &other) const;
	Vec3f Project(const Vec3f &other) const; // Projection of this on to other
};

Vec3f operator*(float scale, const Vec3f &v);
std::ostream &operator<<(std::ostream &output, const Vec3f &v);

Vec3f Lerp(const Vec3f &first, const Vec3f &second, float interpolationCoefficient);