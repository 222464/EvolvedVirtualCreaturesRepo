#include <Renderer/Culler/Frustum.h>

#include <assert.h>

void Frustum::ExtractFromMatrix(const Matrix4x4f &camera)
{
	m_planes[plane_near].Normalized_FromEquationCoeffs(
		camera.Get(0, 2) + camera.Get(0, 3),
		camera.Get(1, 2) + camera.Get(1, 3),
		camera.Get(2, 2) + camera.Get(2, 3),
		camera.Get(3, 2) + camera.Get(3, 3));

	m_planes[plane_far].Normalized_FromEquationCoeffs(
		-camera.Get(0, 2) + camera.Get(0, 3),
		-camera.Get(1, 2) + camera.Get(1, 3),
		-camera.Get(2, 2) + camera.Get(2, 3),
		-camera.Get(3, 2) + camera.Get(3, 3));

	m_planes[plane_bottom].Normalized_FromEquationCoeffs(
		camera.Get(0, 1) + camera.Get(0, 3),
		camera.Get(1, 1) + camera.Get(1, 3),
		camera.Get(2, 1) + camera.Get(2, 3),
		camera.Get(3, 1) + camera.Get(3, 3));

	m_planes[plane_top].Normalized_FromEquationCoeffs(
		-camera.Get(0, 1) + camera.Get(0, 3),
		-camera.Get(1, 1) + camera.Get(1, 3),
		-camera.Get(2, 1) + camera.Get(2, 3),
		-camera.Get(3, 1) + camera.Get(3, 3));

	m_planes[plane_left].Normalized_FromEquationCoeffs(
		camera.Get(0, 0) + camera.Get(0, 3),
		camera.Get(1, 0) + camera.Get(1, 3),
		camera.Get(2, 0) + camera.Get(2, 3),
		camera.Get(3, 0) + camera.Get(3, 3));

	m_planes[plane_right].Normalized_FromEquationCoeffs(
		-camera.Get(0, 0) + camera.Get(0, 3),
		-camera.Get(1, 0) + camera.Get(1, 3),
		-camera.Get(2, 0) + camera.Get(2, 3),
		-camera.Get(3, 0) + camera.Get(3, 3));
}

Frustum::ObjectLocation Frustum::Test_AABB(const AABB &aabb) const
{
	ObjectLocation locGetion = inside;

	// For each plane
	for(int p = 0; p < 6; p++)
	{
		// If positive vertex is outside
		if(m_planes[p].SignedDistanceTo(aabb.GetVertexP(Vec3f(m_planes[p].a, m_planes[p].b, m_planes[p].c))) < 0)
			return outside;

		// If positive vertex is inside
		else if(m_planes[p].SignedDistanceTo(aabb.GetVertexN(Vec3f(m_planes[p].a, m_planes[p].b, m_planes[p].c))) < 0)
			locGetion =  intersect;
	}

	return locGetion;
}

bool Frustum::Test_AABB_Outside(const AABB &aabb) const
{
	ObjectLocation locGetion = inside;

	// For each plane
	for(int p = 0; p < 6; p++)
	{
		// If positive vertex is outside
		if(m_planes[p].SignedDistanceTo(aabb.GetVertexP(Vec3f(m_planes[p].a, m_planes[p].b, m_planes[p].c))) < 0)
			return true;
	}

	return false;
}

void Frustum::CalculateCorners(const Matrix4x4f &cameraInverse)
{
	unsigned int ci = 0;

	for(int x = -1; x < 2; x += 2)
		for(int y = -1; y < 2; y += 2)
			for(int z = -1; z < 2; z += 2)
			{
				Vec4f clipSpaceCoord(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f);

				assert(ci < 8);

				Vec4f homogenous(cameraInverse * clipSpaceCoord);

				m_corners[ci] = Vec3f(homogenous.x / homogenous.w, homogenous.y / homogenous.w, homogenous.z / homogenous.w);

				ci++;
			}
}

Vec3f Frustum::GetCorner(unsigned int index) const
{
	assert(index < 8);

	return m_corners[index];
}