#pragma once

#include <Constructs/Matrix4x4f.h>
#include <Constructs/Vec3f.h>
#include <Constructs/Quaternion.h>

class Camera
{
public:
	Camera();

	Vec3f m_position;

	Quaternion m_rotation;

	void ApplyTransformation();

	void GetViewMatrix(Matrix4x4f &viewMatrix);
};

