#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Vec4f.h>
#include <Constructs/Matrix4x4f.h>

class Quaternion
{
public:
	static const float s_quaternionNormalizationTolerance;
	static const float s_quaternionDotTolerance;

	float w, x, y, z;

	Quaternion();
	Quaternion(float W, float X, float Y, float Z);
	Quaternion(const Vec3f &axis, float angle);
	Quaternion(const Vec3f &eulerAngles);

	float Magnitude() const;
	float MagnitudeSquared() const;
	Quaternion Normalize() const;
	void NormalizeThis();

	Vec4f GetVec4f() const;

	float Dot(const Quaternion &other) const;

	Quaternion operator+(const Quaternion &other) const;
	Quaternion operator-(const Quaternion &other) const;
	
	Quaternion operator*(float scalar) const;
	Quaternion operator/(float scalar) const;

	Quaternion operator*(const Quaternion &other) const;

	const Quaternion &operator+=(const Quaternion &other);
	const Quaternion &operator-=(const Quaternion &other);

	const Quaternion &operator*=(const Quaternion &other);

	const Quaternion &operator*=(float scalar);
	const Quaternion &operator/=(float scalar);

	void Rotate(float angle, const Vec3f &axis);

	static Quaternion GetRotated(float angle, const Vec3f &axis);

	Quaternion Conjugate() const;
	Quaternion Inverse() const;

	void Reset();

	Matrix4x4f GetMatrix() const;

	void SetFromEulerAngles(const Vec3f &eulerAngles);
	Vec3f GetEulerAngles() const;

	void CalculateWFromXYZ();

	void MulMatrix();

	static Quaternion Lerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient);
	static Quaternion Slerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient);
};

Vec3f operator*(const Quaternion &quat, const Vec3f &vec);
Vec3f operator*(const Vec3f &vec, const Quaternion &quat);

std::ostream &operator<<(std::ostream &output, const Quaternion &quat);