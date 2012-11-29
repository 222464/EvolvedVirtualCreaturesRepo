#pragma once

#include <Constructs/Vec3f.h>

#include <Constructs/Matrix4x4f.h>

class AABB
{
private:
	Vec3f m_center;
	Vec3f m_halfDims;

public:
	Vec3f m_lowerBound;
	Vec3f m_upperBound;

	void CalculateHalfDims();
	void CalculateCenter();
	void CalculateBounds();

	// Constructor
	AABB();
	AABB(const Vec3f &lowerBound, const Vec3f &upperBound);

	// Accessors
	const Vec3f &GetCenter() const;

	Vec3f GetDims() const;

	const Vec3f &GetHalfDims() const;
	const Vec3f &GetLowerBound() const;
	const Vec3f &GetUpperBound() const;

	// Modifiers
	void SetLowerBound(const Vec3f &newLowerBound);
	void SetUpperBound(const Vec3f &newUpperBound);
	void SetCenter(const Vec3f &center);
	void IncCenter(const Vec3f &increment);

	void SetDims(const Vec3f &dims);

	void SetHalfDims(const Vec3f &halfDims);

	void Scale(const Vec3f &scale);

	// Utility
	bool Intersects(const AABB &other) const;
	bool Intersects(const Vec3f &p1, const Vec3f &p2) const; // Line segment intersection test
	bool Intersects(const Vec3f &start, const Vec3f &direction, float t[2]) const; // Line intersection test - changes t so point p = start + direction * t[...]. It is an array because it returns 2 points.
	bool Contains(const AABB &other) const;
	bool Contains(const Vec3f &vec) const;

	AABB GetTransformedAABB(const Matrix4x4f &mat) const;

	void DebugRender() const;

	Vec3f GetVertexP(const Vec3f &normal) const;
	Vec3f GetVertexN(const Vec3f &normal) const;

	// Maximum dimension
	float GetRadius() const;

	friend class AABB;
};