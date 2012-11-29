#pragma once

#include <AssetManager/Asset_Texture.h>
#include <AssetManager/Asset.h>

#include <Constructs/Vec2f.h>

#include <Renderer/Model_OBJ.h>

#include <unordered_map>
#include <string>

class WeaponAssetPack :
	public Asset
{
public:
	struct Model_And_Params
	{
		Model_OBJ m_model;
		std::string m_params;

		Model_And_Params()
			: m_params("")
		{
		}
	};

private:
	Asset_Texture m_randomTexture_diffuse;
	Asset_Texture m_randomTexture_normal;
	Asset_Texture m_randomTexture_specular;

	Vec2f m_randomTextureDimsInverse;

	std::unordered_map<std::string, Vec2f> m_randomTextureRegionBias;

	std::unordered_map<std::string, std::vector<Model_And_Params*>> m_categorizedModels;

	float m_swayMultiplier;

	bool m_loaded;
	bool m_renderersSet;

public:
	WeaponAssetPack();
	~WeaponAssetPack();

	// Inherited from Asset
	bool LoadAsset(const std::string &name); // Name of weapon pack descriptor file

	void SetModelRenderers(Scene* pScene);

	Vec2f GetRandomTextureOffset(const std::string &regionName);

	void BindRandomWeaponTextures();

	Model_And_Params* GetRandomModel(const std::string &category);

	const Vec2f &GetRandomTextureDimsInverse();

	// Asset factory
	static Asset* Asset_Factory();
};

