#pragma once

#include <Constructs/Vec2f.h>

#include <Constructs/Matrix4x4f.h>

class AABB2D
{
private:
	Vec2f m_center;
	Vec2f m_halfDims;

public:
	Vec2f m_lowerBound;
	Vec2f m_upperBound;

	void CalculateHalfDims();
	void CalculateCenter();
	void CalculateBounds();

	// Constructor
	AABB2D();
	AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound);

	// Accessors
	const Vec2f &GetCenter() const;

	Vec2f GetDims() const;

	const Vec2f &GetHalfDims() const;
	const Vec2f &GetLowerBound() const;
	const Vec2f &GetUpperBound() const;

	// Modifiers
	void SetLowerBound(const Vec2f &newLowerBound);
	void SetUpperBound(const Vec2f &newUpperBound);
	void SetCenter(const Vec2f &center);
	void IncCenter(const Vec2f &increment);

	void SetDims(const Vec2f &dims);

	void SetHalfDims(const Vec2f &halfDims);

	void Scale(const Vec2f &scale);

	// Utility
	bool Intersects(const AABB2D &other) const;
	bool Contains(const AABB2D &other) const;
	bool Contains(const Vec2f &vec) const;

	AABB2D GetTransformedAABB2D(const Matrix4x4f &mat) const;

	// Maximum dimension
	float GetRadius() const;

	friend class AABB2D;
};