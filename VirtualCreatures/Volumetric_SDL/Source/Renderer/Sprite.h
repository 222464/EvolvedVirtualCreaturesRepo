#pragma once

#include <Scene/SceneObject.h>

#include <AssetManager/Asset_Texture.h>

class Sprite
{
private:
	Asset_Texture* m_pSpriteSheet;

	float m_halfWidth, m_halfHeight;
	float m_texCoordTileWidth, m_texCoordTileHeight;

	int m_numSpritesInX;
	int m_numSprites;

	float m_time;

	float m_spriteFrameTimeInterval;

	float m_animationLength;

public:
	float m_scale;

	Sprite();

	void Create(Asset_Texture* pSpriteSheet, float spriteWidth, int numSpritesInX, int numSpritesInY, int numSprites);

	void SetSpriteFrameTimeInterval(float interval);
	float GetSpriteFrameTimeInterval() const;

	void SetTime(float time);
	void IncTime(float time);

	float GetTime() const;

	void Render();

	float GetHalfWidth() const;
	float GetHalfHeight() const;
};

