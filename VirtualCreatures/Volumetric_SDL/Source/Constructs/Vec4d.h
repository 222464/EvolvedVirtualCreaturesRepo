#pragma once

#include <iostream>

class Vec4d 
{
public:
	double x, y, z, w;

	Vec4d();
	Vec4d(double X, double Y, double Z, double W);
		
	Vec4d operator*(double scale) const;
	Vec4d operator/(double scale) const;
	Vec4d operator+(const Vec4d &other) const;
	Vec4d operator-(const Vec4d &other) const;
	Vec4d operator*(const Vec4d &other) const;
	Vec4d operator/(const Vec4d &other) const;
	Vec4d operator-() const;
		
	const Vec4d &operator*=(double scale);
	const Vec4d &operator/=(double scale);
	const Vec4d &operator+=(const Vec4d &other);
	const Vec4d &operator-=(const Vec4d &other);
	const Vec4d &operator*=(const Vec4d &other);
	const Vec4d &operator/=(const Vec4d &other);

	bool operator==(const Vec4d &other) const;
	bool operator!=(const Vec4d &other) const;
		
	double Magnitude() const;
	double MagnitudeSquared() const;
	void NormalizeThis();
	Vec4d Normalize() const;
	double Dot(const Vec4d &other) const;
};

Vec4d operator*(double scale, const Vec4d &v);
std::ostream &operator<<(std::ostream &output, const Vec4d &v);
