#pragma once

#include <Renderer/BufferObjects/VBO.h>

#include <Constructs/Vec3f.h>

#include <Constructs/Matrix4x4f.h>

#include <vector>

class Wire
{
public:
	struct EndPoint
	{
		Vec3f m_dir;
		Vec3f m_pos;
	};

private:
	bool m_created;

	std::vector<Vec3f> m_points;

	unsigned int m_numVertices;

	VBO m_positionBuffer;
	VBO m_normalBuffer;

	// Could re-use transformed points, but it is easier to just re-transform every time
	void GenerateSegment(const Matrix4x4f &startTransform, const Matrix4x4f &endTransform,
		const Vec3f &startCenter, const Vec3f &endCenter,
		const std::vector<Vec3f> &profile,
		std::vector<Vec3f> &positions, std::vector<Vec3f> &normals);

public:
	Wire();

	bool Generate(unsigned int numSegments, unsigned int coaxialSegments, const EndPoint &start, const EndPoint &end, float radius);

	bool Created();

	void Render();

	void DebugRender();
};

