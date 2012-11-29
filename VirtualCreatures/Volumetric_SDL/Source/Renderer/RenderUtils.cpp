#include <Renderer/RenderUtils.h>

#include <Renderer/SDL_OpenGL.h>

#include <Utilities/UtilFuncs.h>

void DrawSphere(float radius, int res)
{
	float angleSegmentX = pif_times_2 / res;
	float angleSegmentY = pif / res;

	for(int cy = 0; cy < res; cy++)
	{
		float angleY = cy * angleSegmentY - pif_over_2;
		float ringHeightLower = sinf(angleY) * radius;
		float ringHeightHigher = sinf(angleY + angleSegmentY) * radius;

		float ringRadiusLower = cosf(angleY) * radius;
		float ringRadiusHigher = cosf(angleY + angleSegmentY) * radius;

		glBegin(GL_TRIANGLE_STRIP);

		for(int cx = 0; cx <= res; cx ++)
		{
			float angleX = cx * angleSegmentX;

			float cosX = cosf(angleX);
			float sinX = sinf(angleX);

			glVertex3f(cosX * ringRadiusLower, ringHeightLower, sinX * ringRadiusLower);
			glVertex3f(cosX * ringRadiusHigher, ringHeightHigher, sinX * ringRadiusHigher);
		}

		glEnd();
	}
}

int HighestMipMapLevel(int size)
{
	return static_cast<int>(std::min(std::floor(std::log10(static_cast<float>(size)) / std::log10(2.0f)), static_cast<float>(GL_TEXTURE_MAX_LEVEL)));
}

void DrawNormalizedQuad()
{
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	float positions[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f};

	int texCoords[] = {0, 0, 1, 0, 1, 1, 0, 1};

	glVertexPointer(3, GL_FLOAT, 0, positions);
	glTexCoordPointer(2, GL_INT, 0, texCoords);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
}

void DrawQuadOriginCenter(float halfWidth, float halfHeight)
{
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	float positions[] = {
		-halfWidth, -halfHeight, 0.0f,
		halfWidth, -halfHeight, 0.0f,
		halfWidth, halfHeight, 0.0f,
		-halfWidth, halfHeight, 0.0f};

	int texCoords[] = {0, 0, 1, 0, 1, 1, 0, 1};

	glVertexPointer(3, GL_FLOAT, 0, positions);
	glTexCoordPointer(2, GL_INT, 0, texCoords);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
}

void DrawQuadOriginBottomLeft(float width, float height)
{
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_COORD_ARRAY);

	float positions[] = {
		0.0f, 0.0f, 0.0f,
		width, 0.0f, 0.0f,
		width, height, 0.0f,
		0.0f, height, 0.0f};

	int texCoords[] = {0, 0, 1, 0, 1, 1, 0, 1};

	glVertexPointer(3, GL_FLOAT, 0, positions);
	glTexCoordPointer(2, GL_INT, 0, texCoords);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
}