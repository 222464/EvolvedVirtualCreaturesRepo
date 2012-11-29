#pragma once

#include <iostream>

class Vec4f 
{
public:
	float x, y, z, w;

	Vec4f();
	Vec4f(float X, float Y, float Z, float W);
		
	Vec4f operator*(float scale) const;
	Vec4f operator/(float scale) const;
	Vec4f operator+(const Vec4f &other) const;
	Vec4f operator-(const Vec4f &other) const;
	Vec4f operator*(const Vec4f &other) const;
	Vec4f operator/(const Vec4f &other) const;
	Vec4f operator-() const;
		
	const Vec4f &operator*=(float scale);
	const Vec4f &operator/=(float scale);
	const Vec4f &operator+=(const Vec4f &other);
	const Vec4f &operator-=(const Vec4f &other);
	const Vec4f &operator*=(const Vec4f &other);
	const Vec4f &operator/=(const Vec4f &other);

	bool operator==(const Vec4f &other) const;
	bool operator!=(const Vec4f &other) const;
		
	float Magnitude() const;
	float MagnitudeSquared() const;
	void NormalizeThis();
	Vec4f Normalize() const;
	float Dot(const Vec4f &other) const;
};

Vec4f operator*(float scale, const Vec4f &v);
std::ostream &operator<<(std::ostream &output, const Vec4f &v);
