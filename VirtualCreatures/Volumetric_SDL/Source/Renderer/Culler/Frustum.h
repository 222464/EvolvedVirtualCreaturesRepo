#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/AABB.h>
#include <Constructs/Plane_Equationf.h>
#include <Constructs/Matrix4x4f.h>

class Frustum
{
private:

	static enum PlaneOrientation
	{
		plane_top = 0, plane_bottom, plane_left, plane_right, plane_near, plane_far
	};

	Plane_Equationf m_planes[6];

	Vec3f m_corners[8];

public:

	static enum ObjectLocation
	{
		outside, intersect, inside
	};

	void ExtractFromMatrix(const Matrix4x4f &camera); // ViewProjection

	// Tests
	ObjectLocation Test_AABB(const AABB &aabb) const; 
	bool Test_AABB_Outside(const AABB &aabb) const; 

	void CalculateCorners(const Matrix4x4f &cameraInverse); // Inverse ViewProjection
	Vec3f GetCorner(unsigned int index) const;
};

