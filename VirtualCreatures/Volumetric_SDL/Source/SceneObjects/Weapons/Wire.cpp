#include <SceneObjects/Weapons/Wire.h>

#include <Renderer/SDL_OpenGL.h>

#include <Utilities/UtilFuncs.h>

#include <assert.h>

Wire::Wire()
	: m_created(false)
{
}

bool Wire::Generate(unsigned int numSegments, unsigned int coaxialSegments, const EndPoint &start, const EndPoint &end, float radius)
{
	assert(numSegments >= 3);

	// -------------------------------- Compute bezier curve at points --------------------------------

	Vec3f A(start.m_pos);
	Vec3f B(start.m_pos + start.m_dir);
	Vec3f D(end.m_pos);
	Vec3f C(end.m_pos + end.m_dir);

	float increment = 1.0f / static_cast<float>(numSegments);
	const float endT = 1.0f + increment;

	m_points.clear();

	for(float t = 0.0f; t < endT; t += increment)
	{
		float s = 1.0f - t;
		Vec3f AB(A * s + B * t);
		Vec3f BC(B * s + C * t);
		Vec3f CD(C * s + D * t);
		Vec3f ABC(AB * s + CD * t);
		Vec3f BCD(BC * s + CD * t);

		// Get position along curve
		m_points.push_back(ABC * s + BCD * t);
	}

	// -------------------------------- Generate Positions --------------------------------

	// Create the segment profile. Oriented along X axis. Can use normals to get positions, since it is on axis
	std::vector<Vec3f> profilePositions;

	float segmentAngle = pif_times_2 / coaxialSegments;

	for(unsigned int segment = 0; segment < coaxialSegments; segment++)
	{
		float angle = segmentAngle * segment;

		float yOffset = sinf(angle) * radius;
		float zOffset = cosf(angle) * radius;

		profilePositions.push_back(Vec3f(0.0f, yOffset, zOffset));
	}

	std::vector<Vec3f> positions;
	std::vector<Vec3f> normals;

	// Do first segment transform manually
	Matrix4x4f segmentStartTransform(Matrix4x4f::TranslateMatrix(m_points[0]) * Matrix4x4f::DirectionMatrix(start.m_dir.Normalize(), Vec3f(0.0f, 1.0f, 0.0f)));

	const unsigned int sizeTwoLess = m_points.size() - 2;

	for(unsigned int i = 0; i < sizeTwoLess; i++)
	{
		unsigned int nextIndex = i + 1;
		unsigned int nextNextIndex = i + 2;

		Vec3f currentDir(m_points[nextIndex] - m_points[i]);
		Vec3f nextDir(m_points[nextNextIndex] - m_points[nextIndex]);

		Vec3f averageEndDir(currentDir + nextDir);
		averageEndDir.NormalizeThis();

		// Use normal matrix to rotate profile points
		Matrix4x4f segmentEndTransform(Matrix4x4f::TranslateMatrix(m_points[nextIndex]) * Matrix4x4f::DirectionMatrix(averageEndDir, Vec3f(0.0f, 1.0f, 0.0f)));
	
		// Add points of end part of segment
		GenerateSegment(segmentStartTransform, segmentEndTransform, m_points[i], m_points[nextIndex], profilePositions, positions, normals);

		// Carry through next segment
		segmentStartTransform = segmentEndTransform;
	}

	// Do last segment manually
	const unsigned int sizeOneLess = sizeTwoLess + 1;

	// Use normal matrix to rotate profile points. end.m_dir is negated, since it points in the opposite direction of the start-end flow.
	Matrix4x4f segmentEndTransform(Matrix4x4f::TranslateMatrix(m_points[sizeTwoLess]) * Matrix4x4f::DirectionMatrix(-end.m_dir, Vec3f(0.0f, 1.0f, 0.0f)));
	
	// Add points of end part of segment
	GenerateSegment(segmentStartTransform, segmentEndTransform, m_points[sizeTwoLess], m_points[sizeOneLess], profilePositions, positions, normals);

	// -------------------------------- Fill VBO's --------------------------------

	if(!m_created)
	{
		m_positionBuffer.Create();
		m_normalBuffer.Create();
	}

	m_numVertices = positions.size();

	m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * positions.size(), &positions[0], GL_STATIC_DRAW);
	m_positionBuffer.Unbind();

	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	m_normalBuffer.Unbind();

	// ------------------------------ Flag for finish ------------------------------

	m_created = true;

	return true;
}

bool Wire::Created()
{
	return m_created;
}

void Wire::DebugRender()
{
	glBegin(GL_LINES);

	for(unsigned int i = 1, size = m_points.size(); i < size; i++)
	{
		unsigned int prev = i - 1;

		glVertex3f(m_points[prev].x, m_points[prev].y, m_points[prev].z);
		glVertex3f(m_points[i].x, m_points[i].y, m_points[i].z);
	}

	glEnd();
}

void Wire::Render()
{
	assert(m_created);

	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	m_positionBuffer.Bind(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	m_normalBuffer.Bind(GL_ARRAY_BUFFER);
	glNormalPointer(GL_FLOAT, 0, NULL);

	glDrawArrays(GL_QUADS, 0, m_numVertices);

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_NORMAL_ARRAY);
}

void Wire::GenerateSegment(const Matrix4x4f &startTransform, const Matrix4x4f &endTransform, 
	const Vec3f &startCenter, const Vec3f &endCenter,
	const std::vector<Vec3f> &profile,
	std::vector<Vec3f> &positions, std::vector<Vec3f> &normals)
{
	// First start segment manually
	// Start-lower
	Vec3f startPosLower(startTransform * profile[0]);
	Vec3f startNormalLower(startCenter - startPosLower);
	startNormalLower.NormalizeThis();

	// End-lower
	Vec3f endPosLower(endTransform * profile[0]);
	Vec3f endNormalLower(endCenter - endPosLower);
	endNormalLower.NormalizeThis();

	for(unsigned int segmentEnd = 1, size = profile.size(); segmentEnd < size; segmentEnd++)
	{
		// Start-upper
		Vec3f startPosUpper(startTransform * profile[segmentEnd]);
		Vec3f startNormalUpper(startCenter - startPosUpper);
		startNormalUpper.NormalizeThis();

		// End-upper
		Vec3f endPosUpper(endTransform * profile[segmentEnd]);
		Vec3f endNormalUpper(endCenter - endPosUpper);
		endNormalUpper.NormalizeThis();

		// Add vertices
		positions.push_back(startPosLower);
		positions.push_back(endPosLower);
		positions.push_back(endPosUpper);
		positions.push_back(startPosUpper);

		normals.push_back(startNormalLower);
		normals.push_back(endNormalLower);
		normals.push_back(endNormalUpper);
		normals.push_back(startNormalUpper);

		// Carry through
		startPosLower = startPosUpper;
		startNormalLower = startNormalUpper;

		endPosLower = endPosUpper;
		endNormalLower = endNormalUpper;
	}

	// Do last one manually
	// Start-upper
	Vec3f startPosUpper(startTransform * profile[0]);
	Vec3f startNormalUpper(startPosUpper - startCenter);
	startNormalUpper.NormalizeThis();

	// End-upper
	Vec3f endPosUpper(endTransform * profile[0]);
	Vec3f endNormalUpper(endPosUpper - endCenter);
	endNormalUpper.NormalizeThis();

	// Add vertices
	positions.push_back(startPosLower);
	positions.push_back(endPosLower);
	positions.push_back(endPosUpper);
	positions.push_back(startPosUpper);

	normals.push_back(startNormalLower);
	normals.push_back(endNormalLower);
	normals.push_back(endNormalUpper);
	normals.push_back(startNormalUpper);
}