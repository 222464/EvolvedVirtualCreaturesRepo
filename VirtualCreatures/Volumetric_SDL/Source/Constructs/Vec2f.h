#pragma once

#include <iostream>

class Vec2f 
{
public:
	float x, y;

	Vec2f();
	Vec2f(float X, float Y);
		
	Vec2f operator*(float scale) const;
	Vec2f operator/(float scale) const;
	Vec2f operator+(const Vec2f &other) const;
	Vec2f operator-(const Vec2f &other) const;
	Vec2f operator*(const Vec2f &other) const;
	Vec2f operator/(const Vec2f &other) const;
	Vec2f operator-() const;
		
	const Vec2f &operator*=(float scale);
	const Vec2f &operator/=(float scale);
	const Vec2f &operator+=(const Vec2f &other);
	const Vec2f &operator-=(const Vec2f &other);
	const Vec2f &operator*=(const Vec2f &other);
	const Vec2f &operator/=(const Vec2f &other);

	bool operator==(const Vec2f &other) const;
	bool operator!=(const Vec2f &other) const;
		
	float Magnitude() const;
	float MagnitudeSquared() const;
	void NormalizeThis();
	Vec2f Normalize() const;
	float Dot(const Vec2f &other) const;
	Vec2f Project(const Vec2f &other) const; // Projection of this on to other
};

Vec2f operator*(float scale, const Vec2f &v);
std::ostream &operator<<(std::ostream &output, const Vec2f &v);
