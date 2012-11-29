#pragma once

#include <AssetManager/Asset.h>

#include <string>

class Asset_Texture :
	public Asset
{
private:
	unsigned int m_ID;

	int m_width, m_height;
public:
	Asset_Texture();
	~Asset_Texture();

	// Inherited from the asset manager
	bool LoadAsset(const std::string &name);

	unsigned int GetTextureID() const;

	void GenMipMaps();

	void Bind() const;

	int GetWidth() const;
	int GetHeight() const;

	// Asset factory
	static Asset* Asset_Factory();
};

