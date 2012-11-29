#pragma once

#include <Scene/Scene.h>

// Set texture pointers to NULL to indicate it is not used
class Material
{
public:
	Color3f m_diffuseColor;
	float m_specularColor;
	Color3f m_emissiveColor;

	Asset_Texture* m_pDiffuseMap;
	Asset_Texture* m_pSpecularMap;
	Asset_Texture* m_pNormalMap;
	Asset_Texture* m_pEmissiveMap;

	Scene::GBufferRenderShader m_shader;

	Material();

	void Set(Scene* pScene);
	
	static void ResetSceneToDefault(Scene* pScene);

	static bool LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials);
	static bool LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<std::string> &materialNames, std::vector<Material> &materials);
	static bool LoadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::unordered_map<std::string, unsigned int> &materialNamesToIndicesMap, std::vector<Material> &materials);
};