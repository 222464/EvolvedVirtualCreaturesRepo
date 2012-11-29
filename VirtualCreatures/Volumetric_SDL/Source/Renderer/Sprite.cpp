#include <Renderer/Sprite.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

Sprite::Sprite()
	: m_pSpriteSheet(NULL),
	m_spriteFrameTimeInterval(1.0f),
	m_time(0.0f), m_scale(1.0f)
{
}

void Sprite::Create(Asset_Texture* pSpriteSheet, float spriteWidth, int numSpritesInX, int numSpritesInY, int numSprites)
{
	assert(m_pSpriteSheet == NULL);

	m_pSpriteSheet = pSpriteSheet;

	m_halfWidth = spriteWidth / 2.0f;
	m_halfHeight = m_halfWidth * (static_cast<float>(m_pSpriteSheet->GetWidth()) / m_pSpriteSheet->GetHeight());

	m_numSpritesInX = numSpritesInX;
	m_numSprites = numSprites;

	m_texCoordTileWidth = 1.0f / numSpritesInX;
	m_texCoordTileHeight = 1.0f / numSpritesInY;

	m_animationLength = m_spriteFrameTimeInterval * m_numSprites;
}

void Sprite::SetSpriteFrameTimeInterval(float interval)
{
	m_spriteFrameTimeInterval = interval;

	m_animationLength = m_spriteFrameTimeInterval * m_numSprites;
}

float Sprite::GetSpriteFrameTimeInterval() const
{
	return m_spriteFrameTimeInterval;
}

void Sprite::SetTime(float time)
{
	m_time = std::fmodf(time, m_animationLength);

	if(m_time < 0.0f)
		m_time += m_animationLength;
}

void Sprite::IncTime(float time)
{
	m_time = std::fmodf(m_time + time, m_animationLength);

	if(m_time < 0.0f)
		m_time += m_animationLength;
}

float Sprite::GetTime() const
{
	return m_time;
}

void Sprite::Render()
{
	// Get tile
	int tileIndex = static_cast<int>(m_time / m_spriteFrameTimeInterval);

	float texCoordXLower = (tileIndex % m_numSpritesInX) * m_texCoordTileWidth;
	float texCoordYLower = (tileIndex / m_numSpritesInX) * m_texCoordTileHeight;
	float texCoordXHigher = texCoordXLower + m_texCoordTileWidth;
	float texCoordYHigher = texCoordYLower + m_texCoordTileHeight;

	m_pSpriteSheet->Bind();

	float scaledHalfWidth = m_scale * m_halfWidth;
	float scaledHalfHeight = m_scale * m_halfHeight;

	glBegin(GL_QUADS);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(texCoordXLower, texCoordYLower); glVertex3f(0.0f, -scaledHalfHeight, -scaledHalfWidth);
	glTexCoord2f(texCoordXHigher, texCoordYLower); glVertex3f(0.0f, -scaledHalfHeight, scaledHalfWidth);
	glTexCoord2f(texCoordXHigher, texCoordYHigher); glVertex3f(0.0f, scaledHalfHeight, scaledHalfWidth);
	glTexCoord2f(texCoordXLower, texCoordYHigher); glVertex3f(0.0f, scaledHalfHeight, -scaledHalfWidth);
	glEnd();
}

float Sprite::GetHalfWidth() const
{
	return m_halfWidth;
}

float Sprite::GetHalfHeight() const
{
	return m_halfHeight;
}